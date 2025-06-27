# Clustering_Visualize

聚类算法可视化系统

1.系统简介
本系统是一个交互式聚类算法可视化工具，支持多种经典聚类算法，并提供直观的二维数据可视化界面。系统允许用户加载数据集、调整参数、运行聚类算法，并通过动画演示聚类过程。

2.系统功能
支持6种聚类算法：K-means、DBSCAN、层次聚类、谱聚类、Affinity Propagation和DPMM

交互式数据可视化界面

参数可调的聚类过程

聚类过程动画演示

多种辅助信息展示

3.安装指南
系统要求
Linux环境

g++编译器

CMake 3.10或更高版本

Qt6开发环境

Eigen数值计算库

安装步骤
安装依赖库：

bash
sudo apt install g++ cmake qt6-base-dev libeigen3-dev
克隆项目仓库：

bash
git clone https://github.com/voidwarlock/Clustering_Visualize
cd Clustering_Visualize
构建项目：

bash
mkdir build
cd build
cmake ..
make
运行程序：

bash
./Cluster

4.使用说明
主界面介绍


数据加载区：加载和清除数据集

可视化区：显示数据点和聚类结果

算法选择区：选择聚类算法

参数设置区：设置算法参数

控制区：控制动画演示

基本操作流程
加载数据：

点击"Load"按钮

选择.data格式的数据文件

数据点将显示在可视化区

调整视图：

鼠标左键拖动：平移视图

鼠标滚轮：缩放视图

比例调节滑块：调整点的大小

运行聚类：

从算法选择区选择一种聚类算法

在参数设置区输入相应参数

点击"Apply"按钮开始聚类

查看结果：

聚类完成后，数据点将按类别着色

点击"辅助信息"按钮显示额外信息

点击"Display"按钮观看聚类过程动画

调整动画：

使用速度调节滑块控制动画速度

再次点击"Display"暂停动画

第三次点击重置动画

5.数据格式
系统支持.data格式的文本文件，格式要求：

每行一个数据点

每行包含两个浮点数，表示x和y坐标，用空格分隔

示例：

text
1.2 3.4
5.6 7.8
9.0 2.3

6.支持的聚类算法
算法名称	关键参数	特点
K-means	K值	快速简单，需指定簇数
DBSCAN	ε半径，最小点数	基于密度，可发现任意形状簇
层次聚类	簇数或距离阈值	可生成树状图
谱聚类	簇数	适合非凸分布数据
Affinity Propagation	阻尼系数	自动确定簇数
DPMM	α参数	贝叶斯非参数方法

7.常见问题
Q: 加载数据后看不到点？
A: 可能是数据范围过大，尝试：

调整比例滑块

使用鼠标滚轮缩小视图

检查数据文件格式是否正确

Q: 聚类过程卡顿？
A: 对于大数据集或复杂算法(如DPMM)：

减少数据点数量

关闭其他程序释放内存

耐心等待(某些算法复杂度较高)

8.开发者指南
如需扩展系统或修改代码：

新聚类算法应在Cluster.h里添加

可视化组件位于src/目录

核心算法实现在clustering/目录

欢迎提交Pull Request或报告Issue。

9.许可证
本项目采用MIT开源许可证。详细信息请查看LICENSE文件。

特别感谢 https://github.com/datumbox/DPMM-Clustering 中对于DPMM的实现，本代码中的DPMM也是仿照它的结构来实现的
特别感谢 Qwen和Deepseek 对于可视化代码和注释的支持
