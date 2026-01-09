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

BAD_ROWS_LOG = os.path.join(DOC_DIR, "bad_names.txt")
CHANGE_LOG = os.path.join(DOC_DIR, "fixed_names.txt")
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

# --- 智能辅助 ---

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
    try:
        with open(CHANGE_LOG, "a", encoding="utf-8") as f:
            f.write(msg + "\n")
    except: pass

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

# --- 核心自动修复 ---

def attempt_auto_fix_and_verify_block(suspect_row, block_start, block_end):
    """
    针对嫌疑行进行自动修复，并使用整个 Block (block_start~block_end) 进行验证。
    返回: 
      0: 修复成功，且 Block 通过。
      1: 修复成功 (Error 转移)，Block 仍有其他错误。
      -1: 修复失败。
    """
    print(f"\n🔧 [自动修复] 正在尝试修复第 {suspect_row} 行 (Scope: {block_start}-{block_end})...")
    
    orig_name, curr_new_name = get_row_info(suspect_row)
    if not orig_name: return -1

    used_names = get_all_current_names()
    candidates = generate_fix_candidates(orig_name, curr_new_name, used_names)
    
    print(f"  原始: {orig_name} | 当前: {curr_new_name}")
    print(f"  方案: {candidates}")

    for cand in candidates:
        print(f"  👉 尝试方案: {cand} ...")
        
        # 1. 修改 CSV
        update_csv_row(suspect_row, cand)
        
        # 2. 替换代码 (注意：这里替换整个 Block 范围！)
        reset_all_targets()
        if not apply_refactoring_to_all(block_start, block_end):
            continue
            
        # 3. 编译验证 (整体编译)
        success, error_line, error_type = run_command(CMD_BUILD, "验证(整体)", stop_on_error=True)
        
        # 4. 判断结果
        if success:
            log_fix(suspect_row, curr_new_name, cand, orig_name)
            # 完美：不仅修复了嫌疑人，而且整个 Block 都通了
            return 0 
        else:
            # 编译失败，检查是不是同一个错误
            # 我们再次调用 get_smart_suspects 看看现在的报错行是谁
            new_suspects = get_smart_suspects(error_line, block_start, block_end)
            
            # 如果报错行已经不是当前的 suspect_row，说明 suspect_row 修好了！
            # (或者如果 new_suspects 为空，可能是 link 错误，我们假设修好了)
            if not new_suspects or suspect_row not in new_suspects:
                print(f"  ✅ 方案 {cand} 有效！错误已转移 (至 {new_suspects})。")
                log_fix(suspect_row, curr_new_name, cand, orig_name)
                # 保留 CSV 修改，但返回 1，告诉外层继续排查其他错误
                return 1
            else:
                print(f"     ❌ 方案 {cand} 无效，错误仍在第 {suspect_row} 行。")
                # 继续循环下一个 candidate

    # 5. 全部失败
    print(f"  ❌ 所有修复方案均失败。回退到原始名。")
    update_csv_row(suspect_row, orig_name) # 回退 CSV
    return -1

def log_good_range(start, end):
    if start <= end:
        print(f"  >>> ✅ 范围通过: {start}-{end}")
        with open(GOOD_ROWS_LOG, "a", encoding="utf-8") as f:
            f.write(f"{start}-{end}\n")

def log_bad_row(row_num, reason=""):
    print(f"\n>>> ⚠️  锁定坏行: {row_num} {reason}")
    try:
        with open(BAD_ROWS_LOG, "a", encoding="utf-8") as f:
            f.write(f"{row_num}\n")
    except: pass

def check_range(start, end, last_build_success=True):
    if start > end: return

    print(f"\n--- 检查范围: {start} 到 {end} (共 {end - start + 1} 行) ---")
    
    if not apply_refactoring_to_all(start, end):
        reset_all_targets()
        return

    success, error_line, error_type = run_command(CMD_BUILD, "编译检查", stop_on_error=True)

    if not success and error_type in ["LINKER", "FATAL"]:
        if not last_build_success:
            print(f"  [策略] 遇到 {error_type} 且重试阶段 -> 清理并重试...")
            run_clean()
            success, error_line, error_type = run_command(CMD_BUILD, "重试编译", stop_on_error=True)
        else:
            print(f"  [策略] 遇到 {error_type} 但上次成功 -> 判定为代码错误")

    # 智能定位
    suspect_rows = []
    if not success and start != end:
        suspect_rows = get_smart_suspects(error_line, start, end)

    # 此时我们还没有 Reset 代码，正好用于分析 (其实 get_smart_suspects 已经读完了)
    # 但为了后续流程（比如 attempt_auto_fix 里会先 reset 再 apply），这里先不急着 reset
    # 不过上面的 check_range 逻辑里是先 reset 的。
    # 鉴于 attempt_auto_fix 内部会 reset，这里我们可以先 reset 保持一致性
    reset_all_targets()

    if success:
        log_good_range(start, end)
        return

    # === 失败处理 ===
    
    # 1. 锁定单行坏行 -> 尝试修复
    if start == end:
        # 单行模式下，block_start = block_end = start
        res = attempt_auto_fix_and_verify_block(start, start, start)
        if res == 0: # 成功且通过
            log_good_range(start, end)
        elif res == -1: # 修复失败
            log_bad_row(start, f"(无法修复)")
        return

    # 2. 智能定位命中 -> 验证并尝试修复
    if suspect_rows:
        for s_row in suspect_rows:
            # 这里的关键是：用当前大块 (start, end) 来验证 s_row 的修复
            res = attempt_auto_fix_and_verify_block(s_row, start, end)
            
            if res == 0:
                # 完美：修好了 s_row，而且整个 start-end 都通了！
                log_good_range(start, end)
                return 
            
            elif res == 1:
                # 进步：修好了 s_row，但 start-end 还有别的错
                # s_row 已经在 CSV 里被改好了。
                # 我们可以不用急着递归，而是直接在当前层级“继续”二分？
                # 或者更稳健的做法：既然环境变了（CSV变了），我们可以递归二分
                # 此时 s_row 已经是“好人”了。
                print("  [智能分析] 嫌疑人已修复，继续排查剩余错误...")
                # 此时不需要 return，让代码走到下面的二分法
                # 二分法会再次编译，这虽然多了一次编译，但逻辑最简单
                # 优化：如果我们确信错误在另一半，可以直接跳过 s_row
                # 但为了简单，直接 fall through 到 B 步骤即可
                break 

            # res == -1: 没修好，尝试下一个嫌疑人

    # B. 回退二分法
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
        with open(BAD_ROWS_LOG, log_mode, encoding="utf-8") as f:
            if log_mode == "w": f.write(f"Bad Rows Log\n")
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