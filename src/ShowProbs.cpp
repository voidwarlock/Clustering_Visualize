#include "ShowProbs.h"    // 对应的头文件声明
#include <QPainter>       // 用于绘图（可能在 BarChartWidget 中使用）
#include <QFontMetrics>   // 字体度量（可能用于文本绘制计算）
#include <algorithm>      // 算法库（如排序、查找等，当前未直接使用，但可能扩展时用到）
#include <iostream>       // 标准输入输出（调试用）
#include <vector>         // 使用 std::vector 存储数据

/**
 * @brief 构造函数：创建一个显示概率分布的子窗口
 * @param parent 父级 QWidget，默认为 nullptr
 */
SubWindowProbs::SubWindowProbs(QWidget *parent)
    : QDialog(parent) // 继承自 QDialog，作为模态或非模态对话框显示
{
    // 设置窗口标题
    setWindowTitle("Probability Chart");

    // 创建滚动区域，以便在数据过多时支持横向滚动查看图表
    scrollArea_ = new QScrollArea(this);

    // 创建柱状图控件，并将其嵌入滚动区域中
    chartWidget_ = new BarChartWidget(scrollArea_);

    // 将柱状图控件设置为滚动区域的内部部件
    scrollArea_->setWidget(chartWidget_);

    // 设置不自动调整 widget 大小，保持 chartWidget_ 的原始尺寸
    scrollArea_->setWidgetResizable(false);

    // 横向滚动条按需显示，纵向滚动条始终隐藏
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 创建主布局并添加滚动区域
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(scrollArea_);
    setLayout(layout);
}

/**
 * @brief 设置子窗口中要显示的概率数据
 *
 * 将传入的概率值、点坐标和标签传递给底层的柱状图控件，
 * 并根据数据大小动态调整柱状图宽度以适应滚动查看。
 *
 * @param probs 概率值列表（每个点对应一个概率）
 * @param points 对应的二维点坐标列表（可能用于标注或提示）
 * @param labels 对应的标签列表（可用于颜色映射或分类展示）
 */
void SubWindowProbs::setData(const std::vector<double> &probs,
                              const QList<QPointF> &points,
                              const std::vector<int> &labels)
{
    // 将数据传递给柱状图控件进行显示
    chartWidget_->setData(probs, points, labels);

    // 动态调整柱状图控件的宽度：
    // 每个柱宽约为 16 + 22 像素（可能是柱体宽度+间距），乘以总数量得到总宽度
    chartWidget_->resize((16 + 22) * probs.size(), chartWidget_->height());
}