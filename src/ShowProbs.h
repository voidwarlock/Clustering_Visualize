#ifndef SHOWPROBS_H
#define SHOWPROBS_H

// Qt 标准库头文件
#include <QDialog>        // 用于创建子窗口对话框
#include <QScrollArea>    // 提供滚动区域支持，便于显示长内容
#include <QVBoxLayout>    // 垂直布局管理器

// 自定义控件头文件
#include "BarChartWidget.h" // 柱状图绘制控件（需自行实现）

/**
 * @class SubWindowProbs
 * @brief 显示概率分布信息的子窗口类。
 *
 * 该类继承自 QDialog，用于在独立窗口中展示每个数据点对应的概率值。
 * 内部使用 BarChartWidget 绘制柱状图，并通过 QScrollArea 实现横向滚动，
 * 支持查看大量数据点的概率分布情况。
 */
class SubWindowProbs : public QDialog {
    Q_OBJECT // 启用 Qt 的信号与槽机制

public:
    /**
     * 构造函数：初始化子窗口界面
     * @param parent 父级 QWidget，默认为 nullptr
     */
    explicit SubWindowProbs(QWidget *parent = nullptr);

    /**
     * 设置要显示的概率数据及其对应点坐标和标签
     * @param probs 概率值列表（每个点一个 double 类型的概率）
     * @param points 对应的二维点坐标列表（用于标注或提示）
     * @param labels 对应的标签列表（可用于颜色映射或分类展示）
     */
    void setData(const std::vector<double> &probs,
                 const QList<QPointF> &points,
                 const std::vector<int> &labels);

private:
    QScrollArea *scrollArea_;     ///< 滚动区域，用于容纳图表并支持横向滚动
    BarChartWidget *chartWidget_; ///< 实际用于绘制柱状图的自定义控件
};

#endif // SHOWPROBS_H