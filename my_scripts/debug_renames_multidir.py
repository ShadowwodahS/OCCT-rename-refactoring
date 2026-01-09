import subprocess
import os
import sys
import csv
import time
import argparse
import re
import ctypes

# ================= 配置区域 =================

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
DOC_DIR = os.path.join(PROJECT_ROOT, "my_docs")

TARGET_SCAN_DIRS = [
    os.path.join(PROJECT_ROOT, "src"),
    os.path.join(PROJECT_ROOT, "tools"),
]

REFACTOR_SCRIPT_PATH = os.path.join(SCRIPT_DIR, "content_refactor_batch.py")
CSV_PATH = os.path.join(DOC_DIR, "occt_renaming_map.csv")
BAD_ROWS_LOG = os.path.join(DOC_DIR, "bad_renames.txt")
GOOD_ROWS_LOG = os.path.join(DOC_DIR, "good_renames.txt")

SLN_PATH = os.path.normpath(os.path.join(PROJECT_ROOT, "..", "OCCTBUILD", "OCCT.sln"))

CMD_BUILD = [
    "msbuild", SLN_PATH, "/t:Build", "/p:Configuration=Release", 
    "/maxCpuCount", "/p:StopOnFirstFailure=true"
]

CMD_CLEAN = [
    "msbuild", SLN_PATH, "/t:Clean", "/p:Configuration=Release", "/maxCpuCount"
]

CHUNK_SIZE = 50

# ========================================================

def disable_quick_edit():
    if os.name != 'nt': return
    try:
        kernel32 = ctypes.windll.kernel32
        hInput = kernel32.GetStdHandle(-10)
        mode = ctypes.c_ulong()
        if not kernel32.GetConsoleMode(hInput, ctypes.byref(mode)): return
        new_mode = mode.value & ~0x0040
        kernel32.SetConsoleMode(hInput, new_mode)
    except: pass

def count_csv_rows(filepath):
    with open(filepath, 'r', encoding='utf-8-sig') as f:
        reader = csv.reader(f)
        try: next(reader)
        except StopIteration: return 0
        return sum(1 for row in reader)

def kill_process_tree(pid):
    """
    使用 Windows taskkill 命令强制杀死进程及其所有子进程 (/T)
    这是解决 cl.exe 残留的唯一有效办法
    """
    try:
        subprocess.run(
            ["taskkill", "/F", "/T", "/PID", str(pid)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False
        )
    except:
        pass

def nuke_build_processes():
    """
    [核打击] 强制清理系统中所有残留的编译相关进程。
    当 Git 因为文件占用失败时，调用此函数清场。
    """
    targets = ["cl.exe", "link.exe", "vctip.exe", "mspdbsrv.exe", "msbuild.exe"]
    print(f"\n  [系统清理] 正在强制关闭残留进程 ({', '.join(targets)})...", end="")
    for proc in targets:
        subprocess.run(
            ["taskkill", "/F", "/IM", proc],
            stdout=subprocess.DEVNULL, 
            stderr=subprocess.DEVNULL,
            check=False
        )
    print(" 完成。")
    # 给系统一点时间释放句柄
    time.sleep(1.0)

def run_command(cmd, desc, stop_on_error=False):
    print(f"  [执行] {desc}...", end="", flush=True)
    start_time = time.time()
    
    cmd_str = list(map(str, cmd))
    process = None
    captured_error_line = None
    error_type = "NONE"
    log_buffer = []

    try:
        process = subprocess.Popen(
            cmd_str, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            shell=False, text=True, encoding='utf-8', errors='replace'
        )
    except Exception as e:
        print(f"\n  ❌ 启动进程失败: {e}")
        return False, None, "FATAL"

    error_detected = False
    
    while True:
        line = process.stdout.readline()
        if line:
            log_buffer.append(line)
            if stop_on_error:
                line_lower = line.lower()
                is_compiler = "error c" in line_lower
                is_linker = "lnk" in line_lower
                is_fatal = "fatal error" in line_lower
                
                if ": error" in line_lower or is_compiler or is_linker or is_fatal:
                    error_detected = True
                    captured_error_line = line.strip()
                    
                    if is_linker: error_type = "LINKER"
                    elif is_fatal: error_type = "FATAL"
                    else: error_type = "COMPILER"

                    print(f"\n\n{'!'*20} 捕获 {error_type} 错误 {'!'*20}")
                    print(f"信息: {captured_error_line}")
                    print(f"{'!'*54}\n")
                    
                    # 使用加强版 Kill
                    kill_process_tree(process.pid)
                    break
        elif process.poll() is not None:
            break
        else:
            time.sleep(0.05)

    if process and process.poll() is None: 
        # 双重保险
        kill_process_tree(process.pid)
        process.wait()

    duration = time.time() - start_time
    success = (not error_detected) and (process.returncode == 0)
    
    if success:
        print(f" -> ✅ 成功 ({duration:.2f}s)")
    else:
        print(f" -> 🛑 失败 ({duration:.2f}s)")
        if not stop_on_error and process.returncode != 0:
            print("\n" + "="*20 + " 错误日志 " + "="*20)
            print("".join(log_buffer[-20:]))
            print("="*50 + "\n")
        
        # 补救回溯
        if stop_on_error and not error_detected and process.returncode != 0:
             print("\n[警告] 未实时捕获错误，正在回溯日志...")
             for saved_line in reversed(log_buffer):
                 l_low = saved_line.lower()
                 if ": error" in l_low or "error c" in l_low or "fatal error" in l_low:
                     captured_error_line = saved_line.strip()
                     print(f"  >>> 回溯发现: {captured_error_line}")
                     error_type = "UNKNOWN"
                     break
        
    return success, captured_error_line, error_type

def run_clean():
    print("\n  [清理] 执行 MSBuild Clean...")
    # 清理前先杀进程，防止 Clean 被锁
    nuke_build_processes()
    subprocess.run(list(map(str, CMD_CLEAN)), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=False)
    print("  [清理] 完成。")

def reset_all_targets():
    """
    Git 回滚 (加强版：失败时主动查杀僵尸进程)
    """
    for target in TARGET_SCAN_DIRS:
        if not os.path.exists(target): continue
        
        max_retries = 3
        for i in range(max_retries):
            try:
                subprocess.run(["git", "checkout", "HEAD", "--", target], 
                               check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                subprocess.run(["git", "clean", "-fd", target], 
                               check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                break 
            except subprocess.CalledProcessError as e:
                # 如果是最后一次尝试，或者遇到错误，先尝试清理进程
                print(f"\n  [警告] Git 重置被阻止 (尝试 {i+1}/{max_retries})，正在执行核打击清理进程...")
                
                # 关键：杀掉所有可能占用文件的进程
                nuke_build_processes()
                
                if i == max_retries - 1:
                    print(f"\n[致命错误] 即使清理了进程，Git 重置依然失败。")
                    print(f"请检查是否有其他软件（如杀毒软件、文件管理器）占用了目录: {target}")
                    sys.exit(1)

def apply_refactoring_to_all(start, end):
    for target in TARGET_SCAN_DIRS:
        if not os.path.exists(target): continue
        cmd = [
            "python", REFACTOR_SCRIPT_PATH, target, CSV_PATH,
            "--start_row", str(start), "--end_row", str(end), "--run"
        ]
        success, _, _ = run_command(cmd, f"替换 '{os.path.basename(target)}' ({start}-{end})", stop_on_error=False)
        if not success: return False
    return True

def log_bad_row(row_num, reason=""):
    print(f"\n>>> ⚠️  确认坏行: {row_num} {reason}")
    try:
        with open(BAD_ROWS_LOG, "a", encoding="utf-8") as f:
            f.write(f"{row_num}\n")
    except: pass

def log_good_range(start, end):
    if start <= end:
        print(f"  >>> ✅ 确认安全范围: {start}-{end}")
        try:
            with open(GOOD_ROWS_LOG, "a", encoding="utf-8") as f:
                f.write(f"{start}-{end}\n")
        except: pass

def get_csv_range_map(start, end):
    name_to_row = {}
    try:
        with open(CSV_PATH, 'r', encoding='utf-8-sig') as f:
            reader = csv.DictReader(f)
            for i, row in enumerate(reader, start=1):
                if i < start: continue
                if i > end: break
                new_name = row.get('Suggested_New_Name', '').strip()
                if new_name: name_to_row[new_name] = i
    except: pass
    return name_to_row

def extract_word_at_index(text, index):
    if index >= len(text): index = len(text) - 1
    if index < 0: return ""
    if not re.match(r'\w', text[index]):
        while index > 0 and not re.match(r'\w', text[index]): index -= 1
    if not re.match(r'\w', text[index]): return ""
    start = index
    while start > 0 and re.match(r'\w', text[start-1]): start -= 1
    end = index
    while end < len(text)-1 and re.match(r'\w', text[end+1]): end += 1
    return text[start : end+1]

def get_smart_suspects(error_line, start, end):
    if not error_line: return []
    pattern = r"(?:^\d+>)?\s*(.*)\((\d+),(\d+)\)\s*:\s*error"
    match = re.search(pattern, error_line)
    if not match: return []

    file_path = match.group(1).strip()
    line_num = int(match.group(2))
    col_num = int(match.group(3))
    
    if not os.path.exists(file_path): return []

    line_content = ""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
            if line_num <= len(lines):
                line_content = lines[line_num - 1]
    except: return []

    if not line_content: return []

    chunk_map = get_csv_range_map(start, end)
    
    token_tier1 = extract_word_at_index(line_content, col_num - 1)
    if token_tier1 and token_tier1 in chunk_map:
        print(f"  [智能分析] 🎯 精确命中位置 {col_num}: '{token_tier1}'")
        return [(chunk_map[token_tier1], token_tier1)]

    print(f"  [智能分析] 精确位置未命中 (提取词: '{token_tier1}')，转为全行扫描...")
    tokens_in_line = set(re.findall(r"\w+", line_content))
    suspects = []
    for token in tokens_in_line:
        if token in chunk_map:
            suspects.append((chunk_map[token], token))
    
    suspects.sort(key=lambda x: x[0])
    if suspects:
        print(f"  [智能分析] 在行内发现 {len(suspects)} 个嫌疑词: {[s[1] for s in suspects]}")
    
    return suspects

def check_range(start, end, last_build_success=True):
    if start > end: return

    print(f"\n--- 正在检查范围: {start} 到 {end} (共 {end - start + 1} 行) ---")
    
    if not apply_refactoring_to_all(start, end):
        reset_all_targets()
        return

    success, error_line, error_type = run_command(CMD_BUILD, "编译检查", stop_on_error=True)

    if not success and error_type in ["LINKER", "FATAL"]:
        if not last_build_success:
            print(f"  [策略] 遇到 {error_type} 且处于重试阶段 -> 清理并重试...")
            run_clean()
            success, error_line, error_type = run_command(CMD_BUILD, "重试编译", stop_on_error=True)
        else:
            print(f"  [策略] 遇到 {error_type} 但上次编译成功 -> 判定为真实代码错误 (Skip Clean)")

    # 智能定位
    suspects = []
    if not success and start != end:
        suspects = get_smart_suspects(error_line, start, end)

    # 这里的 reset 会尝试暴力清理进程，保证文件解锁
    reset_all_targets()

    if success:
        log_good_range(start, end)
        return

    # === 失败处理 ===
    if start == end:
        log_bad_row(start, f"({error_type} 定位)")
        return

    # A. 智能验证
    if suspects:
        found_culprit = False
        for suspect_row, token_name in suspects:
            print(f"  [验证] 正在验证嫌疑人: {token_name} (Row {suspect_row})...")
            
            if not apply_refactoring_to_all(suspect_row, suspect_row):
                reset_all_targets()
                continue
                
            v_success, _, v_type = run_command(CMD_BUILD, "验证单行", stop_on_error=True)
            
            if not v_success and v_type in ["LINKER", "FATAL"]:
                 run_clean()
                 v_success, _, _ = run_command(CMD_BUILD, "验证单行(Retry)", stop_on_error=True)
            
            reset_all_targets()

            if not v_success:
                log_bad_row(suspect_row, f"(智能锁定: {token_name})")
                if suspect_row > start: check_range(start, suspect_row - 1, last_build_success=False)
                if suspect_row < end: check_range(suspect_row + 1, end, last_build_success=False)
                found_culprit = True
                break 
            else:
                print(f"  [验证] {token_name} 单独编译通过，排除嫌疑。")
        
        if found_culprit: return

    # B. 回退二分法
    mid = (start + end) // 2
    check_range(start, mid, last_build_success=False)
    check_range(mid + 1, end, last_build_success=False)

def main():
    disable_quick_edit()
    parser = argparse.ArgumentParser()
    parser.add_argument("--start_row", type=int, default=1)
    args = parser.parse_args()

    if not os.path.exists(CSV_PATH): return

    log_mode = "w" if args.start_row == 1 else "a"
    try:
        with open(BAD_ROWS_LOG, log_mode, encoding="utf-8") as f:
            if log_mode == "w": f.write(f"Bad Rows Log\n")
        with open(GOOD_ROWS_LOG, log_mode, encoding="utf-8") as f:
            if log_mode == "w": f.write(f"Good Ranges Log\n")
    except: pass

    total_rows = count_csv_rows(CSV_PATH)
    current = args.start_row
    
    print(f"总行数: {total_rows} | 起始: {current} | 步长: {CHUNK_SIZE}")
    print("开始扫描...\n")
    
    while current <= total_rows:
        end = current + CHUNK_SIZE - 1
        if end > total_rows: end = total_rows
        check_range(current, end, last_build_success=True)
        current = end + 1

    print("\n" + "="*60)
    print("扫描完成！")
    print(f"失败行: {os.path.abspath(BAD_ROWS_LOG)}")
    print("="*60)

if __name__ == "__main__":
    main()