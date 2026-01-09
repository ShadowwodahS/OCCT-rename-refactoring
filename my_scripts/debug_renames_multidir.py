import subprocess
import os
import sys
import csv
import time
import argparse
import re
import ctypes
import shutil

# ================= 配置区域 =================

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
DOC_DIR = os.path.join(PROJECT_ROOT, "my_docs")

TARGET_SCAN_DIRS = [
    os.path.join(PROJECT_ROOT, "src"),
    os.path.join(PROJECT_ROOT, "tools"),
]

REFACTOR_SCRIPT_PATH = os.path.join(SCRIPT_DIR, "content_refactor_batch.py")
SOURCE_CSV_PATH = os.path.join(DOC_DIR, "occt_renaming_map.csv")
WORK_CSV_PATH = os.path.join(DOC_DIR, "occt_renaming_map_new.csv")

BAD_ROWS_LOG = os.path.join(DOC_DIR, "bad_names.txt") # 记录修复结果
GOOD_ROWS_LOG = os.path.join(DOC_DIR, "good_renames.txt")

SLN_PATH = os.path.normpath(os.path.join(PROJECT_ROOT, "..", "OCCTBUILD", "OCCT.sln"))

CMD_BUILD = ["msbuild", SLN_PATH, "/t:Build", "/p:Configuration=Release", "/maxCpuCount", "/p:StopOnFirstFailure=true"]
CMD_CLEAN = ["msbuild", SLN_PATH, "/t:Clean", "/p:Configuration=Release", "/maxCpuCount"]

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

def init_work_csv():
    if not os.path.exists(WORK_CSV_PATH):
        shutil.copy2(SOURCE_CSV_PATH, WORK_CSV_PATH)

def count_csv_rows(filepath):
    with open(filepath, 'r', encoding='utf-8-sig') as f:
        reader = csv.reader(f)
        try: next(reader)
        except StopIteration: return 0
        return sum(1 for row in reader)

def kill_process_tree(pid):
    try: subprocess.run(["taskkill", "/F", "/T", "/PID", str(pid)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    except: pass

def nuke_build_processes():
    targets = ["cl.exe", "link.exe", "vctip.exe", "mspdbsrv.exe", "msbuild.exe"]
    for proc in targets:
        subprocess.run(["taskkill", "/F", "/IM", proc], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    time.sleep(0.5)

def run_command(cmd, desc, stop_on_error=False):
    print(f"  [执行] {desc}...", end="", flush=True)
    start_time = time.time()
    cmd_str = list(map(str, cmd))
    process = None
    captured_error_line = None
    error_type = "NONE"
    
    try:
        process = subprocess.Popen(
            cmd_str, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            shell=False, text=True, encoding='utf-8', errors='replace'
        )
    except Exception as e:
        print(f"\n  ❌ 启动失败: {e}")
        return False, None, "FATAL"

    error_detected = False
    
    while True:
        line = process.stdout.readline()
        if line:
            if stop_on_error:
                l_low = line.lower()
                if ": error" in l_low or "error c" in l_low or "fatal error" in l_low:
                    error_detected = True
                    captured_error_line = line.strip()
                    if "lnk" in l_low: error_type = "LINKER"
                    elif "fatal" in l_low: error_type = "FATAL"
                    else: error_type = "COMPILER"
                    print(f"\n\n{'!'*10} 捕获 {error_type} {'!'*10}\n{captured_error_line}\n{'!'*35}")
                    kill_process_tree(process.pid)
                    break
        elif process.poll() is not None:
            break
        else:
            time.sleep(0.05)

    if process and process.poll() is None:
        kill_process_tree(process.pid)
        process.wait()

    duration = time.time() - start_time
    success = (not error_detected) and (process.returncode == 0)
    
    if success: print(f" -> ✅ ({duration:.1f}s)")
    else: print(f" -> 🛑 ({duration:.1f}s)")
    
    return success, captured_error_line, error_type

def run_clean():
    print("\n  [清理] 执行 Clean...")
    nuke_build_processes()
    subprocess.run(list(map(str, CMD_CLEAN)), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=False)

def reset_all_targets():
    for target in TARGET_SCAN_DIRS:
        if not os.path.exists(target): continue
        for i in range(3):
            try:
                subprocess.run(["git", "checkout", "HEAD", "--", target], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                subprocess.run(["git", "clean", "-fd", target], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                break 
            except:
                if i==2: nuke_build_processes()
                time.sleep(1)

def apply_refactoring_to_all(start, end):
    for target in TARGET_SCAN_DIRS:
        if not os.path.exists(target): continue
        cmd = [
            "python", REFACTOR_SCRIPT_PATH, target, WORK_CSV_PATH,
            "--start_row", str(start), "--end_row", str(end), "--run"
        ]
        success, _, _ = run_command(cmd, f"替换 '{os.path.basename(target)}'", stop_on_error=False)
        if not success: return False
    return True

# --- 智能辅助函数 ---

def get_all_current_names():
    names = set()
    with open(WORK_CSV_PATH, 'r', encoding='utf-8-sig') as f:
        reader = csv.DictReader(f)
        for row in reader:
            n = row.get('Suggested_New_Name', '').strip()
            if n: names.add(n)
    return names

def generate_fix_candidates(original_name, current_new_name, used_names):
    candidates = []
    if '_' not in original_name:
        return [original_name] if original_name != current_new_name else []

    parts = original_name.split('_', 1)
    prefix = parts[0]
    suffix = parts[1]
    upper_count = sum(1 for c in suffix if c.isupper())
    
    cand_join = prefix + suffix
    
    base_cand_drop = suffix
    cand_drop = base_cand_drop
    counter = 1
    while cand_drop in used_names and cand_drop != current_new_name:
        cand_drop = f"{base_cand_drop}{counter}"
        counter += 1
    
    if upper_count <= 1:
        candidates.append(cand_join)
        candidates.append(cand_drop)
    else:
        candidates.append(cand_drop)
        candidates.append(cand_join)
    
    candidates.append(original_name)
    
    final_candidates = []
    for c in candidates:
        if c != current_new_name and c not in final_candidates:
            final_candidates.append(c)
    return final_candidates

def update_csv_row(row_num, new_name):
    rows = []
    header = []
    with open(WORK_CSV_PATH, 'r', encoding='utf-8-sig', newline='') as f:
        reader = csv.reader(f)
        header = next(reader)
        rows = list(reader)
    idx = row_num - 1
    if 0 <= idx < len(rows):
        col_idx = header.index('Suggested_New_Name')
        rows[idx][col_idx] = new_name
        with open(WORK_CSV_PATH, 'w', encoding='utf-8-sig', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(header)
            writer.writerows(rows)

def get_row_info(row_num):
    with open(WORK_CSV_PATH, 'r', encoding='utf-8-sig') as f:
        reader = csv.DictReader(f)
        for i, row in enumerate(reader, start=1):
            if i == row_num:
                return row['Original_Class_Name'], row['Suggested_New_Name']
    return None, None

def log_fix(row_num, old_name, fix_name, original_name):
    msg = f"Row {row_num}: FIX [{old_name}] -> [{fix_name}] (Orig: {original_name})"
    print(f"\n✅ {msg}")
    with open(CHANGE_LOG, "a", encoding="utf-8") as f:
        f.write(msg + "\n")

def attempt_auto_fix(row_num):
    print(f"\n🔧 [自动修复] 正在尝试修复第 {row_num} 行...")
    orig_name, curr_new_name = get_row_info(row_num)
    if not orig_name: return False

    used_names = get_all_current_names()
    candidates = generate_fix_candidates(orig_name, curr_new_name, used_names)
    
    print(f"  原始: {orig_name} | 当前: {curr_new_name}")
    print(f"  方案: {candidates}")

    for cand in candidates:
        print(f"  👉 尝试方案: {cand} ...")
        update_csv_row(row_num, cand)
        reset_all_targets() # 清理旧代码
        if not apply_refactoring_to_all(row_num, row_num): continue
        success, _, _ = run_command(CMD_BUILD, "验证修复", stop_on_error=True)
        if success:
            log_fix(row_num, curr_new_name, cand, orig_name)
            reset_all_targets()
            return True
        else:
            print(f"     ❌ 方案失败。")
            reset_all_targets()

    # 修复失败，回退到原始名（放弃治疗）
    update_csv_row(row_num, orig_name)
    with open(CHANGE_LOG, "a", encoding="utf-8") as f:
        f.write(f"Row {row_num}: FAILED FIX. Reverted to {orig_name}\n")
    return False

# --- 智能定位相关 ---

def get_csv_range_map(start, end):
    name_to_row = {}
    try:
        with open(WORK_CSV_PATH, 'r', encoding='utf-8-sig') as f:
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
        print(f"  [智能分析] 🎯 命中位置: '{token_tier1}' (Row {chunk_map[token_tier1]})")
        return [chunk_map[token_tier1]]

    print(f"  [智能分析] 精确未命中 (提取: '{token_tier1}')，转全行扫描...")
    tokens_in_line = set(re.findall(r"\w+", line_content))
    suspects = []
    for token in tokens_in_line:
        if token in chunk_map:
            suspects.append(chunk_map[token])
    
    suspects.sort()
    if suspects:
        print(f"  [智能分析] 行内嫌疑人: {suspects}")
    
    return suspects

def log_good_range(start, end):
    if start <= end:
        print(f"  >>> ✅ 范围通过: {start}-{end}")
        with open(GOOD_ROWS_LOG, "a", encoding="utf-8") as f:
            f.write(f"{start}-{end}\n")

# --- 主逻辑 ---

def check_range(start, end, last_build_success=True):
    if start > end: return

    print(f"\n--- 检查范围: {start} 到 {end} (共 {end - start + 1} 行) ---")
    
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
            print(f"  [策略] 遇到 {error_type} 但上次成功 -> 判定为代码错误")

    # === 关键：先做智能定位，再重置代码！===
    suspect_rows = []
    if not success and start != end:
        suspect_rows = get_smart_suspects(error_line, start, end)
    # ========================================

    reset_all_targets()

    if success:
        log_good_range(start, end)
        return

    # === 失败处理 ===
    
    # 1. 范围缩小到单行 -> 直接修复
    if start == end:
        print(f"⚠️  锁定坏行: {start}")
        attempt_auto_fix(start)
        return

    # 2. 智能定位命中 -> 尝试修复嫌疑人
    if suspect_rows:
        found_real_culprit = False
        for s_row in suspect_rows:
            print(f"  [验证] 正在排查嫌疑行: {s_row} ...")
            # 这里调用 attempt_auto_fix，它内部会进行 替换->编译->回滚
            # 如果修复成功，返回 True
            if attempt_auto_fix(s_row):
                found_real_culprit = True
                # 修复了一个，可能还有别的，但为了安全，我们递归检查
                # 注意：已经修复的行，在 CSV 里变成了新名字，后续 check_range 会使用新名字再次验证，理应通过
                if s_row > start: check_range(start, s_row - 1, last_build_success=False)
                if s_row < end: check_range(s_row + 1, end, last_build_success=False)
                return

        if not found_real_culprit:
            print("  [智能分析] 所有嫌疑行修复尝试均无效/无需修复。回退二分法。")

    # 3. 回退二分法
    mid = (start + end) // 2
    check_range(start, mid, last_build_success=False)
    check_range(mid + 1, end, last_build_success=False)

def main():
    disable_quick_edit()
    parser = argparse.ArgumentParser()
    parser.add_argument("--start_row", type=int, default=1)
    args = parser.parse_args()

    if not os.path.exists(SOURCE_CSV_PATH): return
    init_work_csv()
    
    log_mode = "w" if args.start_row == 1 else "a"
    try:
        with open(CHANGE_LOG, log_mode, encoding="utf-8") as f:
            if log_mode == "w": f.write(f"Fix Log\n")
        with open(GOOD_ROWS_LOG, log_mode, encoding="utf-8") as f:
            if log_mode == "w": f.write(f"Good Ranges Log\n")
    except: pass

    total_rows = count_csv_rows(WORK_CSV_PATH)
    current = args.start_row
    
    print(f"总行数: {total_rows} | 起始: {current} | 步长: {CHUNK_SIZE}")
    
    while current <= total_rows:
        end = current + CHUNK_SIZE - 1
        if end > total_rows: end = total_rows
        check_range(current, end, last_build_success=True)
        current = end + 1

    print("\n扫描完成！")

if __name__ == "__main__":
    main()