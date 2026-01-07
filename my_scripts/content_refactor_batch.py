import os
import argparse
import sys
import re
import csv
import time

# ================= 配置区域 =================

# 1. 允许扫描的文件后缀 (白名单)
ALLOWED_EXTENSIONS = {
    '.hxx', '.hpp', '.cxx', '.cpp', '.c', '.h', '.lxx', '.gxx',
    '.cmake', '.txt', '.mm', '.pxx'
}

# 2. 忽略的目录
IGNORE_DIRS = {
    '.git', '.vs', 'build', 'out', 'bin', 'lib', 
    'tests', 'data', 'samples', 'doc', 'adm'
}

# ===========================================

def load_replacements_from_csv(csv_path, start_row=1, end_row=None):
    """
    读取CSV文件，返回指定行范围内的替换字典。
    自动处理 OCCT 的 Handle_ 前缀情况。
    """
    replacements = {}
    print(f"[*] 正在读取 CSV: {csv_path}")
    print(f"[*] 目标行范围: {start_row} 到 {end_row if end_row else '末尾'}")
    
    try:
        with open(csv_path, 'r', encoding='utf-8') as f:
            reader = csv.DictReader(f)
            
            if 'Original_Class_Name' not in reader.fieldnames or \
               'Suggested_New_Name' not in reader.fieldnames:
                print("[错误] CSV 文件缺少必要的列头: 'Original_Class_Name' 或 'Suggested_New_Name'")
                sys.exit(1)

            count_loaded = 0
            
            for i, row in enumerate(reader, start=1):
                if i < start_row:
                    continue
                if end_row is not None and i > end_row:
                    break

                old_name = row['Original_Class_Name'].strip()
                new_name = row['Suggested_New_Name'].strip()
                
                if old_name and new_name and old_name != new_name:
                    # 1. 添加原始类名替换规则
                    replacements[old_name] = new_name
                    
                    # 2. 自动添加 Handle_ 前缀规则 (OCCT 特性)
                    # 只有当原始名称看起来像个类名（不含空格）时才添加
                    if ' ' not in old_name:
                        handle_old = f"Handle_{old_name}"
                        handle_new = f"Handle_{new_name}"
                        # 避免 CSV 中显式定义了 Handle 规则导致冲突（虽然概率很低）
                        if handle_old not in replacements:
                            replacements[handle_old] = handle_new
                    
                    count_loaded += 1
                    
    except Exception as e:
        print(f"[错误] 读取 CSV 失败: {e}")
        sys.exit(1)
        
    # 注意：这里统计的是 CSV 行数，实际生成的规则数会比这个多（因为包含了 Handle_）
    print(f"[*] 成功加载规则: {count_loaded} 条 (CSV行) -> 扩展为 {len(replacements)} 个替换词条")
    return replacements

def build_regex_pattern(replacements):
    """
    将所有旧名称编译成一个巨大的正则模式。
    """
    if not replacements:
        print("[警告] 没有加载到任何替换规则！请检查行号范围或CSV内容。")
        return None

    print("[*] 正在编译正则表达式...")
    
    # 按照长度降序排列，确保长词优先匹配
    sorted_keys = sorted(replacements.keys(), key=len, reverse=True)
    
    # 转义特殊字符
    escaped_keys = [re.escape(k) for k in sorted_keys]
    
    # 构建核心正则: (WordA|WordB|WordC)
    union_pattern = '|'.join(escaped_keys)
    
    # 组合完整正则：
    # \b          : 单词起始边界
    # (...)       : 捕获组1
    # \b          : 单词结束边界
    # (?!\.(?:...): 负向先行断言，排除 .hxx/.cxx 后缀前的情况
    full_pattern = r'\b(' + union_pattern + r')\b(?!\.(?:hxx|cxx))'
    
    try:
        return re.compile(full_pattern)
    except re.error as e:
        print(f"[错误] 正则表达式编译失败 (可能规则太多导致溢出): {e}")
        sys.exit(1)

def process_file_batch(filepath, regex_pattern, replacements, dry_run):
    """
    处理单个文件：一次性替换所有匹配项
    """
    if regex_pattern is None:
        return False

    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"  [错误] 无法读取 {filepath}: {e}")
        return False

    # 检测 #include 的正则
    include_regex = re.compile(r'^\s*#\s*include')

    new_lines = []
    file_changed = False
    total_replace_count = 0

    # 回调函数
    def replace_callback(match):
        old_word = match.group(1) 
        return replacements.get(old_word, old_word)

    for line in lines:
        if include_regex.match(line):
            new_lines.append(line)
            continue
        
        new_line, count = regex_pattern.subn(replace_callback, line)
        
        if count > 0:
            file_changed = True
            total_replace_count += count
        
        new_lines.append(new_line)

    if file_changed:
        if not dry_run:
            try:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.writelines(new_lines)
                print(f"  [修改] {os.path.basename(filepath)} (共替换 {total_replace_count} 处)")
            except Exception as e:
                print(f"  [写入错误] {filepath}: {e}")
        else:
            print(f"  [预演] {os.path.basename(filepath)} (发现 {total_replace_count} 处匹配)")
            
    return file_changed

def main(root_dir, csv_path, start_row, end_row, dry_run=True):
    start_time = time.time()
    
    # 1. 加载指定行范围的规则
    replacements = load_replacements_from_csv(csv_path, start_row, end_row)
    
    if not replacements:
        print("停止执行：没有规则可供处理。")
        return

    regex_pattern = build_regex_pattern(replacements)

    print(f"[*] 扫描目录: {root_dir}")
    print(f"[*] 限制条件: 全字匹配 | 忽略 #include | 忽略 .hxx/.cxx 后缀前")
    
    if dry_run:
        print("[!] 模式: 预演 (Dry Run) - 不会修改文件")
    else:
        print("[!] 模式: 执行 (Run) - 将修改文件")
    print("-" * 60)

    modified_files_count = 0
    scanned_files_count = 0

    # 2. 遍历目录
    for dirpath, dirnames, filenames in os.walk(root_dir):
        # 过滤目录
        dirnames[:] = [d for d in dirnames if d not in IGNORE_DIRS]

        for filename in filenames:
            file_path = os.path.join(dirpath, filename)
            _, ext = os.path.splitext(filename)

            if ext.lower() not in ALLOWED_EXTENSIONS:
                continue
            
            if filename == os.path.basename(__file__):
                continue
            
            scanned_files_count += 1

            if process_file_batch(file_path, regex_pattern, replacements, dry_run):
                modified_files_count += 1

    end_time = time.time()
    duration = end_time - start_time

    print("-" * 60)
    print(f"处理完成。")
    print(f"耗时: {duration:.2f} 秒")
    print(f"扫描文件: {scanned_files_count}")
    print(f"涉及修改: {modified_files_count}")
    
    if dry_run:
        print("\n提示: 这里的输出仅为预演。请添加 --run 参数来执行实际修改。")
    else:
        print("\n提示: 修改已完成。")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="批量智能代码内容重命名工具 (基于CSV)")
    parser.add_argument("dir", help="源码目录")
    parser.add_argument("csv_file", help="映射表CSV文件路径")
    
    # 添加行号参数
    parser.add_argument("--start_row", type=int, default=1, help="CSV起始行号(包含), 默认为1")
    parser.add_argument("--end_row", type=int, default=None, help="CSV结束行号(包含), 默认为末尾")
    
    parser.add_argument("--run", action="store_true", help="执行实际修改 (默认仅预演)")
    
    args = parser.parse_args()

    if not os.path.exists(args.dir):
        print(f"目录不存在: {args.dir}")
        sys.exit(1)
        
    if not os.path.exists(args.csv_file):
        print(f"CSV文件不存在: {args.csv_file}")
        sys.exit(1)

    main(args.dir, args.csv_file, args.start_row, args.end_row, dry_run=not args.run)