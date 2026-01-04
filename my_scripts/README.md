content_refactor.py：扫描正文中出现的变量名A，替换成B。

batch_runner.py：批量调用content_refactor.py。需要有批量替换规则的csv文件。

generate_new_names.py：生成对应的新类名。注意这个脚本是ai自动生成的，生成时放在了根目录下，识别csv的路径也用的相对路径。