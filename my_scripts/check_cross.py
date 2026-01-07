import csv
from collections import defaultdict

def check_cross_column_collisions(filename):
    # 定义列索引
    COL_ORIGINAL = 1
    COL_NEW = 2

    # 字典：名称 -> 出现的行号列表
    # orig_map: 记录哪些行把这个名字作为“原名” (Original_Class_Name)
    orig_map = defaultdict(list)
    # new_map:  记录哪些行把这个名字作为“新名” (Suggested_New_Name)
    new_map = defaultdict(list)

    # 存储具体的映射关系，方便打印详情: line_num -> (orig, new)
    row_data = {}

    try:
        with open(filename, mode='r', encoding='utf-8', newline='') as csvfile:
            reader = csv.reader(csvfile)
            header = next(reader, None) # 跳过表头

            for line_num, row in enumerate(reader, start=2):
                if len(row) <= COL_NEW:
                    continue

                orig_name = row[COL_ORIGINAL].strip()
                new_name = row[COL_NEW].strip()

                if not orig_name or not new_name:
                    continue

                # 记录数据
                orig_map[orig_name].append(line_num)
                new_map[new_name].append(line_num)
                row_data[line_num] = (orig_name, new_name)

    except FileNotFoundError:
        print(f"错误: 找不到文件 '{filename}'")
        return

    # --- 分析冲突 ---
    # 找出交集：既出现在“原名”列，又出现在“新名”列的名字
    collision_names = set(orig_map.keys()) & set(new_map.keys())

    print(f"正在扫描跨列命名冲突: {filename} ...\n")

    if not collision_names:
        print("✅ 安全！没有发现跨列同名的情况。")
        print("   这意味着没有出现：A->B 且 B->C 的链式情况，也没有 A->B 但 B 依然存在的情况。")
        return

    print(f"⚠️ 发现 {len(collision_names)} 个潜在的跨列命名冲突！")
    print("这表示某个名字既被用作了'新名字'，又是另一个类的'旧名字'。")
    print("=" * 100)
    print(f"{'冲突名称':<30} | {'作为 [新名字] (来源)':<30} | {'作为 [旧名字] (被重命名为)'}")
    print("-" * 100)

    for name in sorted(collision_names):
        # 找出是谁把这个名字当成了新名字
        new_usage_lines = new_map[name]
        # 找出是谁原本就叫这个名字
        orig_usage_lines = orig_map[name]

        # 格式化输出
        # 左边：A -> [NAME] (Line X)
        # 右边：[NAME] -> B (Line Y)
        
        # 收集左边的情况 (生成该名字的行)
        sources = []
        for line in new_usage_lines:
            src_orig, _ = row_data[line]
            if src_orig == name:
                 # 这是一个 A->A 的原地重命名（通常无害，或者是为了API一致性）
                 sources.append(f"自行保持 (Row {line})")
            else:
                 sources.append(f"{src_orig} (Row {line})")
        
        # 收集右边的情况 (消耗该名字的行)
        targets = []
        for line in orig_usage_lines:
            _, tgt_new = row_data[line]
            if tgt_new == name:
                # 这是 A->A
                targets.append(f"自行保持 (Row {line})")
            else:
                targets.append(f"{tgt_new} (Row {line})")

        # 打印
        # 为了排版整齐，如果有多行，分行打印
        max_rows = max(len(sources), len(targets))
        for i in range(max_rows):
            col1 = name if i == 0 else ""
            col2 = sources[i] if i < len(sources) else ""
            col3 = targets[i] if i < len(targets) else ""
            
            # 标记是否是同一行的 A->A
            arrow = " <--> "
            if "自行保持" in col2 and "自行保持" in col3:
                arrow = " (==) " # 自引用，通常安全
            elif "自行保持" in col2 or "自行保持" in col3:
                 pass
            else:
                arrow = " !!!! " # 真正的链式冲突或抢占

            print(f"{col1:<30} | {col2:<30} | {col3}")
            
        print("-" * 100)

if __name__ == "__main__":
    check_cross_column_collisions(r'../my_docs/occt_renaming_map.csv')