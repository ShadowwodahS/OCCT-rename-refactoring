import csv
from collections import defaultdict

def clean_and_check_csv(input_filename, output_filename):
    # 定义列索引
    COL_ORIGINAL_NAME = 1
    COL_NEW_NAME = 2
    
    # 1. 存储去重后的行
    unique_rows = []
    # 2. 用于判断完全重复的集合 (Original_Name, New_Name)
    seen_pairs = set()
    # 3. 用于判断命名冲突的字典 New_Name -> List of Original_Names
    collision_map = defaultdict(list)
    
    removed_count = 0

    try:
        with open(input_filename, mode='r', encoding='utf-8', newline='') as infile:
            reader = csv.reader(infile)
            header = next(reader, None)
            
            if header:
                unique_rows.append(header) # 保留表头

            for line_num, row in enumerate(reader, start=2):
                if len(row) <= COL_NEW_NAME:
                    # 防止空行或格式错误的行导致崩溃，原样保留
                    unique_rows.append(row)
                    continue

                original_name = row[COL_ORIGINAL_NAME].strip()
                new_name = row[COL_NEW_NAME].strip()
                
                # --- 逻辑 1: 自动去重 ---
                # 组合键：(原类名, 新类名)
                pair_key = (original_name, new_name)
                
                if pair_key in seen_pairs:
                    # 如果这对组合已经出现过，说明是完全重复行 -> 跳过（删除）
                    removed_count += 1
                    continue
                else:
                    # 第一次出现 -> 加入集合并保留该行
                    seen_pairs.add(pair_key)
                    unique_rows.append(row)
                    
                    # --- 准备逻辑 2: 记录映射关系用于后续冲突检查 ---
                    # 只有保留下来的行才需要检查冲突
                    if new_name: # 忽略新名字为空的情况
                        collision_map[new_name].append({
                            'orig': original_name,
                            'line': line_num # 记录原始行号，方便查找
                        })

        # 写入清洗后的文件
        with open(output_filename, mode='w', encoding='utf-8', newline='') as outfile:
            writer = csv.writer(outfile)
            writer.writerows(unique_rows)

        print(f"✅ 处理完成！")
        print(f"   - 原始行数: {line_num}")
        print(f"   - 删除完全重复行: {removed_count} 行")
        print(f"   - 剩余有效数据已保存至: {output_filename}\n")

        # --- 逻辑 2: 打印命名冲突 ---
        print(f"🔍 开始检查命名冲突 (不同的类被重命名为同一个名字)...")
        print("-" * 80)
        print(f"{'Suggested_New_Name (冲突的新名)':<35} | {'Original_Class_Name (来源类名)'}")
        print("-" * 80)
        
        conflict_found = False
        for new_name, sources in collision_map.items():
            # sources 是一个列表，包含多个 {'orig': ..., 'line': ...}
            # 如果列表长度 > 1，说明有多个不同的原名映射到了同一个新名
            if len(sources) > 1:
                # 进一步检查：有时候 Original_Name 也是一样的（已经被上面的去重逻辑过滤了，但为了保险）
                # 我们关心的是 Original_Name 是否真的不同
                unique_source_names = set(s['orig'] for s in sources)
                
                if len(unique_source_names) > 1:
                    conflict_found = True
                    print(f"🔴 {new_name:<35} | 映射自多个不同的类:")
                    for src in sources:
                        print(f"{'':<35} |   - {src['orig']} (原第 {src['line']} 行)")
                    print("-" * 80)

        if not conflict_found:
            print("✅ 完美！在去重后的数据中，没有发现命名冲突。")
        else:
            print("⚠️ 警告：发现上述命名冲突，请在生成的cleaned文件中手动修正。")

    except FileNotFoundError:
        print(f"错误: 找不到文件 '{input_filename}'")
    except Exception as e:
        print(f"发生错误: {e}")

if __name__ == "__main__":
    # 输入文件（你的原始csv）
    input_csv = r'../my_docs/occt_renaming_map.csv'
    # 输出文件（清洗后的csv）
    output_csv = 'occt_renaming_map_cleaned.csv'
    
    clean_and_check_csv(input_csv, output_csv)