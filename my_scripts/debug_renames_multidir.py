import subprocess
import os
import sys
import csv
import time
import argparse
import re
import ctypes

# ================= 配置区域 =================

TARGET_SCAN_DIRS = [
    r"..\src", 
    r"..\tools",
]

REFACTOR_SCRIPT_PATH = r"..\my_scripts\content_refactor_batch.py"
CSV_PATH = r"..\my_docs\occt_renaming_map.csv"

# 编译命令 (开启 /p:StopOnFirstFailure=true)
CMD_BUILD = ["msbuild", r"..\..\OCCTBUILD\OCCT.sln", "/t:Build", "/p:Configuration=Release", "/maxCpuCount", "/p:StopOnFirstFailure=true"]

BAD_ROWS_LOG = "bad_renames.txt"
GOOD_ROWS_LOG = "good_renames.txt"
CHUNK_SIZE = 50

# ===========================================

def disable_quick_edit():
    if os.name != 'nt': return
    kernel32 = ctypes.windll.kernel32
    hInput = kernel32.GetStdHandle(-10)
    mode = ctypes.c_ulong()
    if not kernel32.GetConsoleMode(hInput, ctypes.byref(mode)): return
    new_mode = mode.value & ~0x0040
    kernel32.SetConsoleMode(hInput, new_mode)

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

    try:
        process = subprocess.Popen(
            cmd_str, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            shell=False, text=True, encoding='utf-8', errors='replace'
        )
    except Exception as e:
        print(f"\n  启动进程失败: {e}")
        return False, None

    error_detected = False
    
    while True:
        if process.poll() is not None: break
        
        line = process.stdout.readline()
        if line:
            if stop_on_error:
                line_lower = line.lower()
                if ": error" in line_lower or "error c" in line_lower or "fatal error" in line_lower:
                    error_detected = True
                    captured_error_line = line.strip()
                    # 打印错误用于调试，但不用太夸张
                    # print(f"\n[捕获错误] {captured_error_line}")
                    process.kill()
                    break
        time.sleep(0.05)

    if process and process.poll() is None: process.wait()

    duration = time.time() - start_time
    success = (not error_detected) and (process.returncode == 0)
    
    if success: print(f" -> ✅ 成功 ({duration:.2f}s)")
    else: print(f" -> 🛑 失败 ({duration:.2f}s)")
        
    return success, captured_error_line

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
        cmd = [
            "python", REFACTOR_SCRIPT_PATH, target, CSV_PATH,
            "--start_row", str(start), "--end_row", str(end), "--run"
        ]
        success, _ = run_command(cmd, f"替换 '{target}' ({start}-{end})", stop_on_error=False)
        if not success: return False
    return True

def log_bad_row(row_num, reason=""):
    print(f"\n>>> ⚠️  确认坏行: {row_num} {reason}")
    with open(BAD_ROWS_LOG, "a", encoding="utf-8") as f:
        f.write(f"{row_num}\n")

def log_good_range(start, end):
    if start <= end:
        print(f"  >>> ✅ 确认安全范围: {start}-{end}")
        with open(GOOD_ROWS_LOG, "a", encoding="utf-8") as f:
            f.write(f"{start}-{end}\n")

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
    """尝试从错误信息反查行号"""
    if not error_line: return None, None
    pattern = r"^\s*(.*)\((\d+),(\d+)\)\s*:\s*error"
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
    """递归分治 + 验证式智能定位"""
    if start > end: return

    print(f"\n--- 正在检查范围: {start} 到 {end} (共 {end - start + 1} 行) ---")
    
    # 1. 尝试编译整个块
    if not apply_refactoring_to_all(start, end):
        print("  替换脚本执行出错，跳过。")
        reset_all_targets()
        return

    success, error_line = run_command(CMD_BUILD, "编译检查", stop_on_error=True)
    reset_all_targets() # 无论结果如何，先清理环境

    if success:
        log_good_range(start, end)
        return

    # === 编译失败，进入排查阶段 ===

    # 特殊情况：如果只剩一行且失败了，那就是它
    if start == end:
        log_bad_row(start, "(二分定位)")
        return

    # A. 尝试智能定位 + 验证 (Verify)
    suspect_row, token_name = try_smart_detection(error_line, start, end)
    
    if suspect_row:
        print(f"  [智能分析] 怀疑是第 {suspect_row} 行 ({token_name})，开始单独验证...")
        
        # --- 验证步骤 ---
        if not apply_refactoring_to_all(suspect_row, suspect_row):
            reset_all_targets()
        else:
            verify_success, _ = run_command(CMD_BUILD, f"验证单行 {suspect_row}", stop_on_error=True)
            reset_all_targets()

            if not verify_success:
                # 验证失败！实锤了，这行单独编译都过不了
                log_bad_row(suspect_row, f"(智能定位+验证: {token_name})")
                
                # 既然抓到了一个，我们把这一行挖掉，检查剩下的
                # 检查 [start, suspect-1]
                if suspect_row > start:
                    check_range(start, suspect_row - 1)
                # 检查 [suspect+1, end]
                if suspect_row < end:
                    check_range(suspect_row + 1, end)
                
                # 任务完成，退出当前层递归
                return 
            else:
                print(f"  [智能分析] 第 {suspect_row} 行单独编译通过，那是误判或组合错误。转入二分法。")

    # B. 智能分析无效 或 验证未通过（False Positive） -> 回退到二分法
    mid = (start + end) // 2
    check_range(start, mid)
    check_range(mid + 1, end)

def main():
    disable_quick_edit()
    parser = argparse.ArgumentParser()
    parser.add_argument("--start_row", type=int, default=1)
    args = parser.parse_args()

    if not os.path.exists(CSV_PATH): return

    log_mode = "w" if args.start_row == 1 else "a"
    with open(BAD_ROWS_LOG, log_mode, encoding="utf-8") as f:
        if log_mode == "w": f.write(f"Bad Rows\n")
    with open(GOOD_ROWS_LOG, log_mode, encoding="utf-8") as f:
        if log_mode == "w": f.write(f"Good Ranges\n")

    total_rows = count_csv_rows(CSV_PATH)
    current = args.start_row
    
    print(f"总行数: {total_rows} | 步长: {CHUNK_SIZE}")
    
    while current <= total_rows:
        end = current + CHUNK_SIZE - 1
        if end > total_rows: end = total_rows
        check_range(current, end)
        current = end + 1

    print("\n扫描完成。")

if __name__ == "__main__":
    main()