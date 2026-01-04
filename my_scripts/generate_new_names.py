import csv
import os

# Read the renaming map
renaming_map = {}
with open('my_docs/occt_renaming_map.csv', 'r', encoding='utf-8') as f:
    reader = csv.DictReader(f)
    for row in reader:
        key = (row['Original_Package'], row['Original_Class_Name'])
        renaming_map[key] = row['Suggested_New_Name']

# Read the main CSV
rows = []
with open('my_docs/1_occt_build_step1_clean.csv', 'r', encoding='utf-8') as f:
    reader = csv.DictReader(f)
    rows = list(reader)

# Collect all Class Names and New Names to check uniqueness
class_names = set()
new_names = set()
for row in rows:
    class_names.add(row['Class Name'])

# Generate New Names
for row in rows:
    package = row['Package']
    class_name = row['Class Name']
    key = (package, class_name)
    if key in renaming_map:
        new_name = renaming_map[key]
    else:
        # Default: remove package prefix
        if class_name.startswith(package + '_'):
            new_name = class_name[len(package) + 1:]
        else:
            new_name = class_name

    # Ensure uniqueness
    original_new_name = new_name
    counter = 1
    while new_name in new_names or new_name in class_names:
        new_name = f"{original_new_name}{counter}"
        counter += 1
    new_names.add(new_name)
    row['New Name'] = new_name

# Write back
with open('my_docs/1_occt_build_step1_clean.csv', 'w', newline='', encoding='utf-8') as f:
    fieldnames = ['Order', 'Level', 'Toolkit', 'Package', 'Class Name', 'Note', 'New Name']
    writer = csv.DictWriter(f, fieldnames=fieldnames)
    writer.writeheader()
    writer.writerows(rows)

print("Done")