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
        if process.poll() is not None: break
        
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
                    process.kill()
                    break
        time.sleep(0.05)

    if process and process.poll() is None: process.wait()

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
        
    return success, captured_error_line, error_type

def run_clean():
    print("\n  [清理] 执行 MSBuild Clean...")
    subprocess.run(list(map(str, CMD_CLEAN)), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=False)
    print("  [清理] 完成。")

def reset_all_targets():
    for target in TARGET_SCAN_DIRS:
        if not os.path.exists(target): continue
        try:
            subprocess.run(["git", "checkout", "HEAD", "--", target], 
                           check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            subprocess.run(["git", "clean", "-fd", target], 
                           check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except: sys.exit(1)

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
    """返回 {NewName: RowNum}"""
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

def try_smart_detection_full_line(error_line, start, end):
    """
    智能定位 v2.0: 扫描报错行的所有单词，返回所有嫌疑人列表
    返回: [(row_num, token_name), ...]
    """
    if not error_line: return []
    
    # 1. 提取文件路径和行号
    pattern = r"(?:^\d+>)?\s*(.*)\((\d+),(\d+)\)\s*:\s*error"
    match = re.search(pattern, error_line)
    
    if not match: return []

    file_path = match.group(1).strip()
    line_num = int(match.group(2))
    
    if not os.path.exists(file_path): return []

    # 2. 读取整行内容
    line_content = ""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
            if line_num <= len(lines):
                line_content = lines[line_num - 1]
    except: return []

    if not line_content: return []

    # 3. 提取行内所有单词
    # \w+ 匹配所有字母数字下划线组合
    tokens_in_line = set(re.findall(r"\w+", line_content))
    
    if not tokens_in_line: return []

    # 4. 与 CSV 当前块取交集
    chunk_map = get_csv_range_map(start, end)
    suspects = []
    
    for token in tokens_in_line:
        if token in chunk_map:
            suspects.append((chunk_map[token], token))
    
    # 按行号排序，或者按单词长度排序均可，这里按行号
    suspects.sort(key=lambda x: x[0])
    
    if suspects:
        print(f"  [智能分析] 在报错行发现 {len(suspects)} 个可疑词: {[s[1] for s in suspects]}")
    
    return suspects

def check_range(start, end):
    if start > end: return

    print(f"\n--- 正在检查范围: {start} 到 {end} (共 {end - start + 1} 行) ---")
    
    # 1. 替换
    if not apply_refactoring_to_all(start, end):
        reset_all_targets()
        return

    # 2. 编译
    success, error_line, error_type = run_command(CMD_BUILD, "编译检查", stop_on_error=True)

    # 3. 智能清理策略
    if not success and error_type in ["LINKER", "FATAL"]:
        print(f"  [策略] 遇到 {error_type}，清理并重试...")
        run_clean()
        success, error_line, error_type = run_command(CMD_BUILD, "重试编译", stop_on_error=True)

    reset_all_targets()

    # 4. 成功分支
    if success:
        log_good_range(start, end)
        return

    # === 失败分支 ===
    
    if start == end:
        log_bad_row(start, f"({error_type} 定位)")
        return

    # A. 全行扫描智能定位
    suspects = try_smart_detection_full_line(error_line, start, end)
    
    # 遍历所有嫌疑人
    found_culprit = False
    
    for suspect_row, token_name in suspects:
        print(f"  [验证] 正在验证嫌疑人: {token_name} (Row {suspect_row})...")
        
        # 验证单行
        if not apply_refactoring_to_all(suspect_row, suspect_row):
            reset_all_targets()
            continue # 脚本执行错，跳过
            
        v_success, _, v_type = run_command(CMD_BUILD, "验证单行", stop_on_error=True)
        
        if not v_success and v_type in ["LINKER", "FATAL"]:
            run_clean()
            v_success, _, _ = run_command(CMD_BUILD, "验证单行(Retry)", stop_on_error=True)
            
        reset_all_targets()

        if not v_success:
            # 抓到了！
            log_bad_row(suspect_row, f"(全行扫描锁定: {token_name})")
            
            # 分裂递归：跳过这个坏人
            # 只要找到一个，我们就拆分递归，因为可能有多个错误交织
            # 策略：拆分为 [start, bad-1] 和 [bad+1, end]
            # 注意：一旦递归，当前的 check_range 就任务完成了
            if suspect_row > start: check_range(start, suspect_row - 1)
            if suspect_row < end: check_range(suspect_row + 1, end)
            
            found_culprit = True
            break # 退出嫌疑人循环，因为已经进入了下一层递归
        else:
            print(f"  [验证] {token_name} 单独编译通过，排除嫌疑。")

    if found_culprit:
        return # 已由内部递归接管

    # B. 如果所有嫌疑人都无罪释放（或者没找到嫌疑人）-> 回退二分法
    print("  [智能分析] 未能锁定具体行，回退到二分查找。")
    mid = (start + end) // 2
    check_range(start, mid)
    check_range(mid + 1, end)

def main():
    disable_quick_edit()
    parser = argparse.ArgumentParser(description="OCCT 智能重命名排查工具 (全行扫描版)")
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
        check_range(current, end)
        current = end + 1

    print("\n" + "="*60)
    print(f"失败行: {os.path.abspath(BAD_ROWS_LOG)}")
    print(f"成功块: {os.path.abspath(GOOD_ROWS_LOG)}")
    print("="*60)

if __name__ == "__main__":
    main()