#ifndef BARCHARTWIDGET_H
#define BARCHARTWIDGET_H

// 确保头文件仅被包含一次。虽然 #pragma once 也用于此目的，但通常与#ifndef一起使用以增加兼容性。
#pragma once 
#include <QWidget> // 包含 QWidget 类定义，用于创建自定义控件
#include <vector>  // 使用 std::vector 容器
#include <QList>   // 使用 QList 容器
#include <QPointF> // 使用 QPointF 表示点坐标

/**
 * @class BarChartWidget
 * 自定义的 Qt 控件，用于绘制柱状图。
 * 支持设置数据、标签和附加点信息，并基于这些信息进行绘图。
 */
class BarChartWidget : public QWidget {
    Q_OBJECT // 必须在继承自 QObject 的类中声明（如 QWidget），以启用信号和槽机制

public:
    /**
     * 构造函数
     * 初始化 BarChartWidget 实例。
     * @param parent 父级 widget，默认为 nullptr。如果指定父级，则该 widget 成为其子级。
     */
    explicit BarChartWidget(QWidget *parent = nullptr);

    /**
     * 设置或更新柱状图的数据。
     * @param data 柱状图每个柱子对应的数值（概率）。
     * @param points 与每个柱子关联的 (x, y) 数据点。
     * @param labels 对应于每个柱子的标签，用于颜色映射或其他用途。
     */
    void setData(const std::vector<double> &data, const QList<QPointF> &points, const std::vector<int> &labels);

protected:
    /**
     * 重写的 paintEvent 函数，用于绘制柱状图。
     * 当需要重新绘制部件时由框架调用。
     * @param event 绘制事件对象，描述了绘制请求的信息。
     */
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<double> data_; // 存储柱状图中每个柱子的高度（概率）
    QList<QPointF> points_;    // 存储每个柱子相关的 (x, y) 数据点
    std::vector<int> labels_;  // 存储每个柱子对应的标签，可用于颜色映射等
    double maxProb_ = 1.0;     // 最大概率值，默认初始化为 1.0，实际值会在 setData 中计算并更新
};

#endif // BARCHARTWIDGET_H