import subprocess
import os
import sys
import csv
import time
import argparse
import ctypes

def disable_quick_edit():
    """
    禁用 Windows 控制台的 '快速编辑模式'，防止鼠标点击导致进程挂起。
    """
    if os.name != 'nt':
        return

    # Windows API 常量
    ENABLE_QUICK_EDIT_MODE = 0x0040
    STD_INPUT_HANDLE = -10
    
    kernel32 = ctypes.windll.kernel32
    
    # 获取标准输入句柄
    hInput = kernel32.GetStdHandle(STD_INPUT_HANDLE)
    
    # 获取当前控制台模式
    mode = ctypes.c_ulong()
    if not kernel32.GetConsoleMode(hInput, ctypes.byref(mode)):
        return

    # 移除快速编辑模式位
    new_mode = mode.value & ~ENABLE_QUICK_EDIT_MODE
    
    # 设置新模式
    kernel32.SetConsoleMode(hInput, new_mode)
    print("  [系统] 已自动禁用控制台'快速编辑模式'，防止鼠标误触挂起。")

# 1. 指定要扫描和替换的目标源码目录列表
TARGET_SCAN_DIRS = [
    r"..\src", 
    r"..\tools",
]

# 2. 脚本与数据路径
REFACTOR_SCRIPT_PATH = r"..\my_scripts\content_refactor_batch.py"
CSV_PATH = r"..\my_docs\occt_renaming_map.csv"

# 3. 编译命令
CMD_BUILD = ["msbuild", r"..\..\OCCTBUILD\OCCT.sln", "/t:Build", "/p:Configuration=Release", "/maxCpuCount",
    "/p:StopOnFirstFailure=true"]

# 4. 日志文件路径
BAD_ROWS_LOG = "bad_renames.txt"
GOOD_ROWS_LOG = "good_renames.txt"

# 5. 初始扫描步长
CHUNK_SIZE = 50

# ===========================================

def count_csv_rows(filepath):
    """计算CSV总行数"""
    with open(filepath, 'r', encoding='utf-8-sig') as f:
        reader = csv.reader(f)
        try:
            next(reader)
        except StopIteration:
            return 0
        return sum(1 for row in reader)

def run_command(cmd, desc, stop_on_error=False):
    """运行命令 (主动监控进程状态)"""
    print(f"  [执行] {desc}...", end="", flush=True)
    
    start_time = time.time()
    cmd_str = list(map(str, cmd))
    
    process = None
    try:
        process = subprocess.Popen(
            cmd_str, 
            stdout=subprocess.PIPE, 
            stderr=subprocess.STDOUT,
            shell=False, 
            text=True,
            encoding='utf-8', 
            errors='replace'
        )
    except Exception as e:
        print(f"\n  启动进程失败: {e}")
        return False

    error_detected = False
    stop_reason = ""
    
    while True:
        if process.poll() is not None:
            break 
        
        line = process.stdout.readline()
        if line:
            if stop_on_error:
                line_lower = line.lower()
                if (": error" in line_lower or "error c" in line_lower or "错误 c" in line_lower or "fatal error" in line_lower):
                    error_detected = True
                    stop_reason = line.strip()
                    print(f"\n\n{'!'*20} 捕获编译错误 {'!'*20}")
                    print(f"错误信息: {stop_reason}")
                    print(f"{'!'*54}\n")
                    process.kill() 
                    break 
        time.sleep(0.05) 

    if process and process.poll() is None:
        process.wait()

    duration = time.time() - start_time
    
    if error_detected:
        print(f" -> 🛑 编译失败 (已终止, 耗时 {duration:.2f}s)")
        return False
    elif process and process.returncode != 0:
        print(f" -> ❌ 执行失败 (返回码 {process.returncode}, 耗时 {duration:.2f}s)")
        return False
    else:
        print(f" -> ✅ 成功 ({duration:.2f}s)")
        return True

def reset_all_targets():
    """Git 回滚与清理"""
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
    """执行批量替换"""
    for target in TARGET_SCAN_DIRS:
        cmd = [
            "python", REFACTOR_SCRIPT_PATH, target, CSV_PATH,
            "--start_row", str(start), "--end_row", str(end), "--run"
        ]
        if not run_command(cmd, f"替换 '{target}' ({start}-{end})", stop_on_error=False):
            return False
    return True

def log_bad_row(row_num):
    """记录失败行"""
    print(f"\n>>> ⚠️  锁定问题行: {row_num}")
    with open(BAD_ROWS_LOG, "a", encoding="utf-8") as f:
        f.write(f"{row_num}\n")

def log_good_range(start, end):
    """记录成功块"""
    if start <= end:
        print(f"  >>> ✅ 确认安全范围: {start}-{end}")
        with open(GOOD_ROWS_LOG, "a", encoding="utf-8") as f:
            f.write(f"{start}-{end}\n")

def check_range(start, end):
    """递归分治检查"""
    print(f"\n--- 正在检查范围: {start} 到 {end} (共 {end - start + 1} 行) ---")
    
    # 1. 替换
    if not apply_refactoring_to_all(start, end):
        print("  替换脚本出错，跳过此块。")
        reset_all_targets()
        return

    # 2. 编译
    build_success = run_command(CMD_BUILD, "编译检查", stop_on_error=True)

    # 3. 回滚
    reset_all_targets()

    # 4. 逻辑判断
    if build_success:
        log_good_range(start, end)
        return
    else:
        # 失败处理
        if start == end:
            log_bad_row(start)
        else:
            mid = (start + end) // 2
            check_range(start, mid)   # 查前半部分
            check_range(mid + 1, end) # 查后半部分

def main():
    disable_quick_edit()

    # 1. 解析参数
    parser = argparse.ArgumentParser(description="OCCT 重命名二分排查工具")
    parser.add_argument("--start_row", type=int, default=1, help="指定起始行号 (默认为 1)")
    args = parser.parse_args()

    # 2. 路径检查
    if not os.path.exists(CSV_PATH):
        print(f"错误: 找不到 CSV {CSV_PATH}")
        return
    if not os.path.exists(REFACTOR_SCRIPT_PATH):
        print(f"错误: 找不到脚本 {REFACTOR_SCRIPT_PATH}")
        return

    # 3. 智能初始化日志文件
    # 如果是从第1行开始，说明是新的一轮，覆盖旧日志 ('w')
    # 如果指定了 start_row > 1，说明是续传，使用追加模式 ('a')，防止丢失之前的记录
    log_mode = "w" if args.start_row == 1 else "a"
    
    with open(BAD_ROWS_LOG, log_mode, encoding="utf-8") as f:
        if log_mode == "w": f.write(f"Bad Rows (Failures)\n")
        
    with open(GOOD_ROWS_LOG, log_mode, encoding="utf-8") as f:
        if log_mode == "w": f.write(f"Good Ranges (Success)\n")

    total_rows = count_csv_rows(CSV_PATH)
    
    # 设定起始行
    current_row = args.start_row
    if current_row < 1: current_row = 1
    if current_row > total_rows:
        print(f"错误: 起始行 {current_row} 超过了 CSV 总行数 {total_rows}")
        return

    print(f"总行数: {total_rows} | 扫描起始行: {current_row} | 步长: {CHUNK_SIZE}")
    print(f"日志模式: {'覆盖 (Overwrite)' if log_mode == 'w' else '追加 (Append)'}")
    print("开始扫描...\n")

    while current_row <= total_rows:
        end_row = current_row + CHUNK_SIZE - 1
        if end_row > total_rows:
            end_row = total_rows
        
        check_range(current_row, end_row)
        current_row = end_row + 1

    print("\n" + "="*60)
    print("扫描完成！结果已保存：")
    print(f"1. 失败行号: {os.path.abspath(BAD_ROWS_LOG)}")
    print(f"2. 成功范围: {os.path.abspath(GOOD_ROWS_LOG)}")
    print("="*60)

if __name__ == "__main__":
    main()