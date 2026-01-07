1. 重命名有可能导致类名和原本的函数名相同。这个是没有检查的。比如gp_Dir，ai推荐的重命名是Direction3d，但OCCT原本有同名的函数，会导致编译失败。当然，理论上，应该把函数名改为GetDirection3d，不过我尝试了一下修改，依旧有奇怪的失败。只能先放弃这个尝试，gp_Dir重命名成Dir3d了。
2. 更新1: Dir3d这个名字也不行，放弃重命名了。保留原名gp_Dir。
3. ShapeExplorer，这个命名和原本的局部变量名相同。把局部变量改成小写开头了。
4. 最早的Vs Code给的命名，是用脚本生成的，其实保证了New Name必定不重复的。但后面借助了Grok等其他ai的命名，导致这个名字出现重复了。再次写了个check_duplicates.py单独检测重复。
5. Transition1是TopOpeBRep_EdgesIntersector的一个方法（方法还有叫这种名字的？），不能重名成这个类。
6. 甚至出现了比4. 更坏的情况，New Name有出现其它Old Name的名字。再次写了个check_cross.py来检测重复。