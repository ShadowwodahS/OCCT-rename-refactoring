import csv
import sys
import os
import subprocess
import argparse
import time

def main():
    parser = argparse.ArgumentParser(
        description="批量执行 content_refactor.py 进行代码重命名",
        formatter_class=argparse.RawTextHelpFormatter  # 保持帮助信息格式
    )
    # --- 位置参数 ---
    parser.add_argument("source_dir", 
                        help="需要被重构的源码目录路径")
    parser.add_argument("csv_file", 
                        help="重命名映射表 CSV 文件路径")
    
    # --- 可选参数 ---
    parser.add_argument("--script_path", 
                        default="content_refactor.py",
                        help="content_refactor.py 脚本的路径 (默认: content_refactor.py)")
    parser.add_argument("--start-row", 
                        type=int, 
                        default=1,
                        help="从 CSV 文件的第几行数据开始执行 (默认: 1)。\n"
                             "注: 这是指数据行，不包括表头。1 代表第一条数据。")
    parser.add_argument("--end-row", 
                        type=int, 
                        default=None,
                        help="执行到 CSV 文件的第几行数据结束 (包含此行)。\n"
                             "默认执行到文件末尾。")
    parser.add_argument("--run", 
                        action="store_true", 
                        help="执行实际修改。如果未指定此参数，脚本将以“预演”模式运行。")
    
    args = parser.parse_args()

    # 1. 检查所有路径的有效性
    if not os.path.isdir(args.source_dir):
        print(f"错误: 源码目录不存在 -> {args.source_dir}")
        sys.exit(1)
    if not os.path.isfile(args.csv_file):
        print(f"错误: CSV文件不存在 -> {args.csv_file}")
        sys.exit(1)
    if not os.path.isfile(args.script_path):
        print(f"错误: 替换脚本不存在 -> {args.script_path}")
        sys.exit(1)

    # 2. 读取 CSV 并构建替换列表
    all_replacements = []
    try:
        with open(args.csv_file, 'r', encoding='utf-8') as f:
            reader = csv.reader(f)
            header = next(reader)  # 跳过标题行
            
            # (row_idx, old_name, new_name)
            for row_idx, row in enumerate(reader, start=1): # 数据行从1开始计数
                if len(row) < 3:
                    print(f"警告: 跳过格式错误的行 (数据行 {row_idx}): {row}")
                    continue
                
                old_name = row[1].strip()
                new_name = row[2].strip()

                if old_name and new_name and old_name != new_name:
                    all_replacements.append((row_idx, old_name, new_name))
    except Exception as e:
        print(f"读取 CSV 失败: {e}")
        sys.exit(1)

    # 3. 根据 start/end 参数筛选要执行的规则
    total_rules = len(all_replacements)
    
    # 校验行号
    if args.start_row <= 0:
        print(f"错误: --start-row 必须是正整数。")
        sys.exit(1)
    if args.end_row is not None and args.end_row < args.start_row:
        print(f"错误: --end-row ({args.end_row}) 不能小于 --start-row ({args.start_row})。")
        sys.exit(1)
        
    start_index = args.start_row - 1
    end_index = args.end_row if args.end_row is not None else total_rules

    if start_index >= total_rules:
        print(f"指定的开始行 ({args.start_row}) 超出总数据行数 ({total_rules})，无需执行。")
        sys.exit(0)

    # 使用 Python 切片获取目标范围
    selected_replacements = all_replacements[start_index:end_index]
    
    if not selected_replacements:
        print("根据指定的行号范围，没有选中任何需要执行的规则。")
        sys.exit(0)

    # 4. 打印执行计划
    print(f"[*] 成功从 '{os.path.basename(args.csv_file)}' 加载 {total_rules} 条有效规则。")
    print(f"[*] 目标目录: {args.source_dir}")
    print(f"[*] 执行模式: {'写入 (Run)' if args.run else '预演 (Dry Run)'}")
    
    effective_start_row = selected_replacements[0][0]
    effective_end_row = selected_replacements[-1][0]
    print(f"[*] 执行范围: 数据行 {effective_start_row} 到 {effective_end_row} (共 {len(selected_replacements)} 条)。")
    print("=" * 70)
    
    # 5. 开始循环调用子脚本
    start_time = time.time()
    for i, (original_row_num, old, new) in enumerate(selected_replacements, 1):
        progress = f"[{i}/{len(selected_replacements)}]"
        print(f"{progress} (CSV行 {original_row_num}) 正在处理: {old} -> {new}")
        
        cmd = [
            sys.executable,
            args.script_path,
            args.source_dir,
            old,
            new
        ]
        
        if args.run:
            cmd.append("--run")
        
        try:
            # text=True, check=True 会在子进程出错时抛出异常
            subprocess.run(cmd, text=True, check=True)
        except subprocess.CalledProcessError as e:
            print(f"  [严重错误] 子脚本执行失败，返回码: {e.returncode}。中止执行。")
            sys.exit(1)
        except Exception as e:
            print(f"  [致命错误] 调用脚本时发生异常: {e}。中止执行。")
            sys.exit(1)

        print("-" * 40)

    elapsed = time.time() - start_time
    print(f"[*] 全部完成。共处理 {len(selected_replacements)} 条规则，耗时: {elapsed:.2f} 秒。")

if __name__ == "__main__":
    main()