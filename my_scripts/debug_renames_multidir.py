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

# 假设 OCCTBUILD 在项目根目录的上一级同级目录中
SLN_PATH = os.path.normpath(os.path.join(PROJECT_ROOT, "..", "OCCTBUILD", "OCCT.sln"))

CMD_BUILD = [
    "msbuild", SLN_PATH, "/t:Build", "/p:Configuration=Release", 
    "/maxCpuCount", "/p:StopOnFirstFailure=true"
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
    """
    运行命令 (带日志缓存与错误回显 - 修复版)
    """
    print(f"  [执行] {desc}...", end="", flush=True)
    start_time = time.time()
    
    cmd_str = list(map(str, cmd))
    
    process = None
    captured_error_line = None
    log_buffer = [] # 用于缓存所有输出日志

    try:
        process = subprocess.Popen(
            cmd_str, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            shell=False, text=True, encoding='utf-8', errors='replace'
        )
    except Exception as e:
        print(f"\n  ❌ 启动进程失败: {e}")
        return False, None

    error_detected = False
    
    while True:
        if process.poll() is not None: break
        
        line = process.stdout.readline()
        if line:
            log_buffer.append(line) # 无论如何都先存起来
            
            if stop_on_error:
                line_lower = line.lower()
                # 关键词匹配
                if ": error" in line_lower or "error c" in line_lower or "fatal error" in line_lower:
                    error_detected = True
                    captured_error_line = line.strip()
                    # 只要检测到关键词，立刻杀进程
                    process.kill()
                    break
        time.sleep(0.05)

    if process and process.poll() is None: process.wait()

    duration = time.time() - start_time
    
    # 判断是否成功：没有检测到错误关键词 且 返回码为0
    success = (not error_detected) and (process.returncode == 0)
    
    if success:
        print(f" -> ✅ 成功 ({duration:.2f}s)")
    else:
        print(f" -> 🛑 失败 ({duration:.2f}s)")
        
        # === 关键修复：打印失败日志 ===
        # 无论是编译错误被 kill，还是其他原因导致的非 0 返回码，都打印日志
        print("\n" + "="*30 + " 错误日志片段 (最后 30 行) " + "="*30)
        
        # 如果捕获到了具体的 error 行，优先打印上下文
        if captured_error_line:
            print(f"捕获到的关键错误:\n>>> {captured_error_line}\n")
            print("--- 上下文 ---")
        
        # 打印最后 30 行日志，通常包含了错误原因
        print("".join(log_buffer[-30:])) 
        print("="*80 + "\n")
        
    return success, captured_error_line

def reset_all_targets():
    for target in TARGET_SCAN_DIRS:
        if not os.path.exists(target): continue
        try:
            subprocess.run(["git", "checkout", "HEAD", "--", target], 
                           check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            subprocess.run(["git", "clean", "-fd", target], 
                           check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except subprocess.CalledProcessError:
            print(f"Git reset failed for {target}")
            sys.exit(1)

def apply_refactoring_to_all(start, end):
    for target in TARGET_SCAN_DIRS:
        if not os.path.exists(target): continue
        cmd = [
            "python", REFACTOR_SCRIPT_PATH, target, CSV_PATH,
            "--start_row", str(start), "--end_row", str(end), "--run"
        ]
        # 注意：这里 stop_on_error=False，因为 python 脚本报错我们希望看到完整 Traceback
        success, _ = run_command(cmd, f"替换 '{os.path.basename(target)}' ({start}-{end})", stop_on_error=False)
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

def try_smart_detection(error_line, start, end):
    if not error_line: return None, None
    pattern = r"(?:^\d+>)?\s*(.*)\((\d+),(\d+)\)\s*:\s*error"
    match = re.search(pattern, error_line)
    if not match: return None, None

    file_path = match.group(1).strip()
    line_num = int(match.group(2))
    col_num = int(match.group(3))

    if not os.path.exists(file_path): return None, None

    token = ""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
            if line_num <= len(lines):
                target_line = lines[line_num - 1]
                if col_num > len(target_line): col_num = 0
                substr = target_line[max(0, col_num-1):] 
                token_match = re.search(r"(\w+)", substr)
                if token_match: token = token_match.group(1)
    except: return None, None

    if not token: return None, None

    chunk_map = get_csv_range_map(start, end)
    if token in chunk_map:
        return chunk_map[token], token
    return None, None

def check_range(start, end):
    if start > end: return

    print(f"\n--- 正在检查范围: {start} 到 {end} (共 {end - start + 1} 行) ---")
    
    # 1. 替换
    if not apply_refactoring_to_all(start, end):
        print("  替换脚本出错，跳过。")
        reset_all_targets()
        return

    # 2. 编译
    success, error_line = run_command(CMD_BUILD, "编译检查", stop_on_error=True)
    reset_all_targets()

    if success:
        log_good_range(start, end)
        return

    # === 编译失败 ===
    if start == end:
        log_bad_row(start, "(二分定位)")
        return

    # 智能分析
    suspect_row, token_name = try_smart_detection(error_line, start, end)
    
    if suspect_row:
        print(f"  [智能分析] 怀疑第 {suspect_row} 行 ({token_name})，开始验证...")
        if not apply_refactoring_to_all(suspect_row, suspect_row):
            reset_all_targets()
        else:
            verify_success, _ = run_command(CMD_BUILD, f"验证单行 {suspect_row}", stop_on_error=True)
            reset_all_targets()

            if not verify_success:
                log_bad_row(suspect_row, f"(智能定位+验证: {token_name})")
                if suspect_row > start: check_range(start, suspect_row - 1)
                if suspect_row < end: check_range(suspect_row + 1, end)
                return 
            else:
                print(f"  [智能分析] 第 {suspect_row} 行单独编译通过，转入二分法。")

    # 回退到二分法
    mid = (start + end) // 2
    check_range(start, mid)
    check_range(mid + 1, end)

def main():
    disable_quick_edit()
    parser = argparse.ArgumentParser(description="OCCT 智能重命名排查工具 (带日志版)")
    parser.add_argument("--start_row", type=int, default=1, help="指定起始行号")
    args = parser.parse_args()

    if not os.path.exists(CSV_PATH):
        print(f"错误: 找不到 CSV {CSV_PATH}")
        return

    log_mode = "w" if args.start_row == 1 else "a"
    try:
        with open(BAD_ROWS_LOG, log_mode, encoding="utf-8") as f:
            if log_mode == "w": f.write(f"Bad Rows Log\n")
        with open(GOOD_ROWS_LOG, log_mode, encoding="utf-8") as f:
            if log_mode == "w": f.write(f"Good Ranges Log\n")
    except: pass

    total_rows = count_csv_rows(CSV_PATH)
    current = args.start_row
    
    if current > total_rows:
        print(f"起始行 {current} 超过总行数 {total_rows}")
        return

    print(f"总行数: {total_rows} | 起始: {current} | 步长: {CHUNK_SIZE}")
    print("开始扫描...\n")
    
    while current <= total_rows:
        end = current + CHUNK_SIZE - 1
        if end > total_rows: end = total_rows
        check_range(current, end)
        current = end + 1

    print("\n" + "="*60)
    print("扫描完成！")
    print(f"失败行: {os.path.abspath(BAD_ROWS_LOG)}")
    print("="*60)

if __name__ == "__main__":
    main()