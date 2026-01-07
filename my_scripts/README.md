content_refactor.py：扫描正文中出现的变量名A，替换成B。

batch_runner.py：批量调用content_refactor.py。需要有批量替换规则的csv文件。

generate_new_names.py：生成对应的新类名。注意这个脚本是ai自动生成的，生成时放在了根目录下，识别csv的路径也用的相对路径。

debug_renames_multidir.py：按步长批量替换，并且尝试编译，看对应行的替换会不会有问题。每次尝试后都会回滚源码。

check_duplicates.py：经过多个ai取名，现在New Name出现有相同的了，需要检查出来。这个脚本还会同时清理完全相同的重复行。

check_cross.py：经过多个ai取名，现在New Name还会和别的Old Name相同，需要检查出来。