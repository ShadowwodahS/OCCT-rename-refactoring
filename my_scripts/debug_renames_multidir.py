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

# 原始 CSV (只读)
SOURCE_CSV_PATH = os.path.join(DOC_DIR, "occt_renaming_map.csv")
# 工作 CSV (读写，脚本将修改此文件)
WORK_CSV_PATH = os.path.join(DOC_DIR, "occt_renaming_map_new.csv")

# 日志
CHANGE_LOG = os.path.join(DOC_DIR, "bad_names.txt") # 记录修复日志
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

def init_work_csv():
    """初始化工作CSV：如果不存在则复制源文件"""
    if not os.path.exists(WORK_CSV_PATH):
        print(f"[初始化] 创建副本: {os.path.basename(WORK_CSV_PATH)}")
        shutil.copy2(SOURCE_CSV_PATH, WORK_CSV_PATH)
    else:
        print(f"[初始化] 使用现有副本: {os.path.basename(WORK_CSV_PATH)}")

def count_csv_rows(filepath):
    with open(filepath, 'r', encoding='utf-8-sig') as f:
        reader = csv.reader(f)
        try: next(reader)
        except StopIteration: return 0
        return sum(1 for row in reader)

# --- 进程管理 ---
def kill_process_tree(pid):
    try:
        subprocess.run(["taskkill", "/F", "/T", "/PID", str(pid)], 
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    except: pass

def nuke_build_processes():
    targets = ["cl.exe", "link.exe", "vctip.exe", "mspdbsrv.exe", "msbuild.exe"]
    for proc in targets:
        subprocess.run(["taskkill", "/F", "/IM", proc], 
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
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
                is_err = ": error" in l_low or "error c" in l_low or "fatal error" in l_low
                if is_err:
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
    """Git 回滚"""
    for target in TARGET_SCAN_DIRS:
        if not os.path.exists(target): continue
        for i in range(3):
            try:
                subprocess.run(["git", "checkout", "HEAD", "--", target], 
                               check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                subprocess.run(["git", "clean", "-fd", target], 
                               check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                break 
            except:
                if i==2: nuke_build_processes()
                time.sleep(1)

def apply_refactoring_to_all(start, end):
    for target in TARGET_SCAN_DIRS:
        if not os.path.exists(target): continue
        # 注意：这里使用 WORK_CSV_PATH
        cmd = [
            "python", REFACTOR_SCRIPT_PATH, target, WORK_CSV_PATH,
            "--start_row", str(start), "--end_row", str(end), "--run"
        ]
        success, _, _ = run_command(cmd, f"替换 '{os.path.basename(target)}'", stop_on_error=False)
        if not success: return False
    return True

# --- 自动修复逻辑 ---

def get_all_current_names():
    """读取工作CSV中所有已存在的 Suggested_New_Name，用于防重"""
    names = set()
    with open(WORK_CSV_PATH, 'r', encoding='utf-8-sig') as f:
        reader = csv.DictReader(f)
        for row in reader:
            n = row.get('Suggested_New_Name', '').strip()
            if n: names.add(n)
    return names

def generate_fix_candidates(original_name, current_new_name, used_names):
    """
    生成候选名字列表 (优先级从高到低)
    规则 1: 模块_单词 -> 模块单词 (去下划线)
    规则 2: 模块_多词 -> 多词 (去模块)
    规则 3: 回退 (Original)
    """
    candidates = []
    
    # 如果没有下划线，直接回退
    if '_' not in original_name:
        return [original_name] if original_name != current_new_name else []

    parts = original_name.split('_', 1)
    prefix = parts[0]
    suffix = parts[1]

    # 判断 suffix 是单个单词还是多个单词 (简单的 CamelCase 判定)
    # 比如 "Solid" -> 1个大写; "ParamCursor" -> 2个大写
    upper_count = sum(1 for c in suffix if c.isupper())
    
    cand_join = prefix + suffix  # 规则1: TopoDSSolid
    
    # 规则2: ParamCursor (需处理冲突)
    base_cand_drop = suffix
    cand_drop = base_cand_drop
    counter = 1
    # 确保不和现有的冲突 (排除掉自己当前的名字，因为我们要改它)
    while cand_drop in used_names and cand_drop != current_new_name:
        cand_drop = f"{base_cand_drop}{counter}"
        counter += 1
    
    # 决策优先级
    if upper_count <= 1:
        # 单单词: 优先 TopoDSSolid，其次 Solid，最后回退
        candidates.append(cand_join)
        candidates.append(cand_drop)
    else:
        # 多单词: 优先 ParamCursor，其次 IGESDataParamCursor，最后回退
        candidates.append(cand_drop)
        candidates.append(cand_join)
    
    candidates.append(original_name) # 兜底：回退

    # 过滤掉和当前失败名字一样的，避免重复试
    final_candidates = []
    for c in candidates:
        if c != current_new_name and c not in final_candidates:
            final_candidates.append(c)
            
    return final_candidates

def update_csv_row(row_num, new_name):
    """更新 CSV 指定行的 Suggested_New_Name"""
    rows = []
    header = []
    with open(WORK_CSV_PATH, 'r', encoding='utf-8-sig', newline='') as f:
        reader = csv.reader(f)
        header = next(reader)
        rows = list(reader)
    
    # row_num 是从1开始的数据行号
    idx = row_num - 1
    if 0 <= idx < len(rows):
        # 假设 Suggested_New_Name 是第3列 (索引2)
        # 根据你的 map1.csv 结构: Original_Package, Original_Class_Name, Suggested_New_Name
        col_idx = header.index('Suggested_New_Name')
        old_val = rows[idx][col_idx]
        rows[idx][col_idx] = new_name
        
        with open(WORK_CSV_PATH, 'w', encoding='utf-8-sig', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(header)
            writer.writerows(rows)
        return old_val
    return None

def get_row_info(row_num):
    """获取指定行的原始信息"""
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
    """
    尝试自动修复坏行
    """
    print(f"\n🔧 [自动修复] 正在尝试修复第 {row_num} 行...")
    
    orig_name, curr_new_name = get_row_info(row_num)
    if not orig_name: return False

    used_names = get_all_current_names()
    candidates = generate_fix_candidates(orig_name, curr_new_name, used_names)
    
    print(f"  原始类名: {orig_name}")
    print(f"  当前失败: {curr_new_name}")
    print(f"  修复方案: {candidates}")

    for cand in candidates:
        print(f"  👉 尝试方案: {cand} ...")
        
        # 1. 修改 CSV
        update_csv_row(row_num, cand)
        
        # 2. 替换代码 (单行)
        reset_all_targets() # 先清理
        if not apply_refactoring_to_all(row_num, row_num):
            continue
            
        # 3. 编译验证 (不 Clean，因为是单行验证，且前面可能clean过了)
        success, _, _ = run_command(CMD_BUILD, "验证修复", stop_on_error=True)
        
        if success:
            log_fix(row_num, curr_new_name, cand, orig_name)
            reset_all_targets() # 成功后清理，准备下一步
            return True
        else:
            print(f"     ❌ 方案 {cand} 失败。")
            reset_all_targets()

    # 如果所有方案都失败（包括回退），那真是没救了
    print(f"  ❌ 所有修复方案均失败 (包括回退)。保留最后一次尝试。")
    # 恢复为回退状态(通常是最后一个candidate)
    update_csv_row(row_num, orig_name)
    with open(CHANGE_LOG, "a", encoding="utf-8") as f:
        f.write(f"Row {row_num}: FAILED TO FIX. Reverted to {orig_name}\n")
    return False

# --- 主流程 ---

def log_good_range(start, end):
    if start <= end:
        print(f"  >>> ✅ 范围通过: {start}-{end}")
        with open(GOOD_ROWS_LOG, "a", encoding="utf-8") as f:
            f.write(f"{start}-{end}\n")

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
            success, _, _ = run_command(CMD_BUILD, "重试编译", stop_on_error=True)
        else:
            print(f"  [策略] 遇到 {error_type} 但上次成功 -> 判定为代码错误")

    reset_all_targets()

    if success:
        log_good_range(start, end)
        return

    # === 失败处理 ===
    
    # 1. 锁定到单行 -> 启动自动修复
    if start == end:
        print(f"⚠️  锁定坏行: {start}")
        attempt_auto_fix(start)
        return

    # 2. 尝试智能定位
    # 注意：get_smart_suspects 在这里无法使用，因为我们已经 reset 了代码，
    # 而且我们不能依赖报错信息反查 New Name (因为我们要改的就是它)。
    # 鉴于我们要修改 CSV，为了逻辑简单稳健，建议直接退化为二分法，
    # 直到缩小到单行，再进行 Auto Fix。
    
    # 如果你想保留智能定位，需要像之前那样在 reset 之前做，
    # 但考虑到现在要改 CSV，二分法虽然慢点但逻辑最安全。
    
    mid = (start + end) // 2
    check_range(start, mid, last_build_success=False)
    check_range(mid + 1, end, last_build_success=False)

def main():
    disable_quick_edit()
    parser = argparse.ArgumentParser()
    parser.add_argument("--start_row", type=int, default=1)
    args = parser.parse_args()

    if not os.path.exists(SOURCE_CSV_PATH):
        print(f"错误: 找不到源 CSV {SOURCE_CSV_PATH}")
        return

    # 初始化工作环境
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

    print("\n" + "="*60)
    print("处理完成！")
    print(f"新 CSV: {os.path.abspath(WORK_CSV_PATH)}")
    print(f"修复日志: {os.path.abspath(CHANGE_LOG)}")
    print("="*60)

if __name__ == "__main__":
    main()