import os
import argparse
import sys
import re

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

def is_text_file(filepath):
    """简单检测文件是否可读（非二进制）"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            f.read(512)
            return True
    except UnicodeDecodeError:
        return False
    except Exception:
        return False

def process_file(filepath, old_word, new_word, dry_run):
    """
    处理单个文件：
    1. 逐行读取
    2. 跳过 #include 行
    3. 对普通行进行全字匹配替换
    """
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"  [错误] 无法读取 {filepath}: {e}")
        return False

    # 1. 编译正则表达式
    # \b 确保全字匹配 (Whole Word Only)
    word_regex = re.compile(r'\b' + re.escape(old_word) + r'\b')
    
    # 检测 #include 的正则 (允许 # 前后有空格)
    include_regex = re.compile(r'^\s*#\s*include')

    new_lines = []
    file_changed = False
    replace_count = 0

    for line in lines:
        # 2. 核心逻辑：如果是 include 行，原样保留，不进行替换
        if include_regex.match(line):
            new_lines.append(line)
            continue
        
        # 3. 普通代码行：执行替换
        new_line, count = word_regex.subn(new_word, line)
        if count > 0:
            file_changed = True
            replace_count += count
        
        new_lines.append(new_line)

    # 4. 如果有改动，写回文件
    if file_changed:
        if not dry_run:
            try:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.writelines(new_lines)
                print(f"  [修改] {os.path.basename(filepath)} (替换 {replace_count} 处)")
            except Exception as e:
                print(f"  [写入错误] {filepath}: {e}")
        else:
            print(f"  [预演] {os.path.basename(filepath)} (发现 {replace_count} 处匹配 - 非include)")
            
    return file_changed

def main(root_dir, old_word, new_word, dry_run=True):
    print(f"[*] 扫描目录: {root_dir}")
    print(f"[*] 替换规则: '{old_word}' -> '{new_word}'")
    print(f"[*] 限制条件: 全字匹配 | 忽略 #include 行 | 不改文件名")
    
    if dry_run:
        print("[!] 模式: 预演 (Dry Run)")
    else:
        print("[!] 模式: 执行 (Run)")
    print("-" * 60)

    modified_files = 0

    for dirpath, dirnames, filenames in os.walk(root_dir):
        # 过滤目录
        dirnames[:] = [d for d in dirnames if d not in IGNORE_DIRS]

        for filename in filenames:
            file_path = os.path.join(dirpath, filename)
            _, ext = os.path.splitext(filename)

            # 过滤后缀
            if ext.lower() not in ALLOWED_EXTENSIONS:
                continue
            
            # 过滤脚本自身
            if filename == os.path.basename(__file__):
                continue

            # 处理文件
            if process_file(file_path, old_word, new_word, dry_run):
                modified_files += 1

    print("-" * 60)
    print(f"处理完成。共修改了 {modified_files} 个文件。")
    if dry_run:
        print("\n提示: 请添加 --run 参数来执行实际修改。")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="智能代码内容替换工具")
    parser.add_argument("dir", help="源码目录")
    parser.add_argument("old_word", help="旧类名 (如 Standard_Transient)")
    parser.add_argument("new_word", help="新类名 (如 Transient)")
    parser.add_argument("--run", action="store_true", help="执行实际修改")
    
    args = parser.parse_args()

    if not os.path.exists(args.dir):
        print("目录不存在")
        sys.exit(1)

    main(args.dir, args.old_word, args.new_word, dry_run=not args.run)