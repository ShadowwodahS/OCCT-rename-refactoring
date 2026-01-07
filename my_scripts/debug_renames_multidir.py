import subprocess
import os
import sys
import csv
import time

# ================= 配置区域 =================

# 1. 指定要扫描和替换的目标源码目录列表 (支持多个)
TARGET_SCAN_DIRS = [
    r"..\src", 
    r"..\tools",
    # r"samples",
]

# 2. 脚本与数据路径
#    注意：请确保 content_refactor.py 已经是那个一次性批量替换的版本
REFACTOR_SCRIPT_PATH = r"..\my_scripts\content_refactor_batch.py"
CSV_PATH = r"..\my_docs\occt_renaming_map.csv"

# 3. 编译命令 (开启多核 /maxCpuCount)
#    建议: 如果觉得 kill 之后还要等很久，可以尝试去掉 "/maxCpuCount" 改成单核编译，响应会更快
CMD_BUILD = ["msbuild", r"..\..\OCCTBUILD\OCCT.sln", "/t:Build", "/p:Configuration=Release", "/maxCpuCount"]

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
    """
    运行命令 (终极版：主动监控进程状态，快速终止并回显错误)
    """
    print(f"  [执行] {desc}...", end="", flush=True)
    
    start_time = time.time()
    cmd_str = list(map(str, cmd))
    
    process = None # 初始化 process 变量
    try:
        process = subprocess.Popen(
            cmd_str, 
            stdout=subprocess.PIPE, 
            stderr=subprocess.STDOUT,
            shell=False, 
            text=True,
            encoding='utf-8', # 尝试 utf-8, 如果乱码再改回 mbcs
            errors='replace'
        )
    except FileNotFoundError:
        print(f"\n  错误: 找不到命令 {cmd[0]}。请确保在 Visual Studio 开发者命令行中运行。")
        return False
    except Exception as e:
        print(f"\n  启动进程失败: {e}")
        return False

    error_detected = False
    stop_reason = ""
    
    # 循环检查进程是否还在运行
    while True:
        # 1. 检查进程是否还活着
        if process.poll() is not None: # poll() 返回 None 表示进程还在运行
            # 进程已退出，检查是否之前就检测到错误
            break 
        
        # 2. 尝试读取一行（如果有的话）
        line = process.stdout.readline()
        if line:
            # print(".", end="", flush=True) # 进度指示 (可选)
            
            # 3. 检查错误关键词
            line_lower = line.lower()
            if stop_on_error and (": error" in line_lower or "error c" in line_lower or "错误 c" in line_lower or "fatal error" in line_lower):
                error_detected = True
                stop_reason = line.strip()
                
                # 打印错误并终止
                print(f"\n\n{'!'*20} 捕获编译错误 {'!'*20}")
                print(f"错误信息: {stop_reason}")
                print(f"{'!'*54}\n")
                
                # 激进终止
                process.kill() 
                break # 退出循环
        
        # 避免CPU空转
        time.sleep(0.05) 

    # 确保进程已完全退出
    if process and process.poll() is None: # 如果上面 break 了但进程还没退出
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
            # 恢复被修改的文件
            subprocess.run(["git", "checkout", "HEAD", "--", target], 
                           check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            # 清理生成的临时文件
            subprocess.run(["git", "clean", "-fd", target], 
                           check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except subprocess.CalledProcessError:
            print(f"Git reset failed for {target}")
            sys.exit(1)

def apply_refactoring_to_all(start, end):
    """执行批量替换"""
    for target in TARGET_SCAN_DIRS:
        # 直接调用内容替换脚本
        cmd = [
            "python", REFACTOR_SCRIPT_PATH, target, CSV_PATH,
            "--start_row", str(start), "--end_row", str(end), "--run"
        ]
        # 注意：这里 stop_on_error=False，因为 python 脚本的报错通常不是 C++ 编译错误格式
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

    # 2. 编译 (这是最耗时的一步，也是需要快速失败的一步)
    build_success = run_command(CMD_BUILD, "编译检查", stop_on_error=True)

    # 3. 回滚 (保持环境纯净)
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
    if not os.path.exists(CSV_PATH):
        print(f"错误: 找不到 CSV {CSV_PATH}")
        return
    if not os.path.exists(REFACTOR_SCRIPT_PATH):
        print(f"错误: 找不到脚本 {REFACTOR_SCRIPT_PATH}")
        return

    # 初始化日志
    with open(BAD_ROWS_LOG, "w", encoding="utf-8") as f:
        f.write(f"Bad Rows (Failures)\n")
    with open(GOOD_ROWS_LOG, "w", encoding="utf-8") as f:
        f.write(f"Good Ranges (Success)\n")

    total_rows = count_csv_rows(CSV_PATH)
    print(f"总行数: {total_rows} | 初始步长: {CHUNK_SIZE}")
    print("开始扫描...\n")

    current_row = 1
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