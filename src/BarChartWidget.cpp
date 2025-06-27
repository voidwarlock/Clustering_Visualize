#include "BarChartWidget.h"
#include <QPainter>
#include <QFontMetrics>
#include "Label2Color.h" // 自定义颜色映射头文件

/**
 * 构造函数：初始化柱状图控件
 * @param parent 父级 QWidget，默认为 nullptr
 */
BarChartWidget::BarChartWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(600); // 设置最小高度为 600 像素
}

/**
 * 设置柱状图数据
 * @param data 概率值数组（每个柱子的高度）
 * @param points 与每个柱子关联的 (x, y) 数据点
 * @param labels 每个柱子对应的标签（用于颜色映射）
 */
void BarChartWidget::setData(const std::vector<double> &data,
                             const QList<QPointF> &points,
                             const std::vector<int> &labels)
{
    data_ = data;       // 存储概率数据
    points_ = points;   // 存储坐标点数据
    labels_ = labels;   // 存储标签数据

    if (!data_.empty()) {
        maxProb_ = *std::max_element(data_.begin(), data_.end()); // 找到最大概率值
    }

    update(); // 触发重绘事件
}

/**
 * 绘制事件处理函数：绘制柱状图及坐标轴、标签、数据点等
 * @param event QPaintEvent 对象（未使用）
 */
void BarChartWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿渲染

    int margin = 50;          // 边距
    int barWidth = 16;        // 柱子宽度
    int spacing = 22;         // 柱子之间的间距
    int chartHeight = height() - 2 * margin; // 图表区域高度
    int chartWidth = (barWidth + spacing) * data_.size(); // 图表区域宽度

    // 根据图表内容设置控件的最小宽度，以支持水平滚动
    setMinimumWidth(chartWidth + 2 * margin);

    // 清除背景为白色
    painter.fillRect(rect(), Qt::white);

    // 绘制坐标轴 X 和 Y
    painter.setPen(Qt::black);
    painter.drawLine(margin, height() - margin, margin + chartWidth, height() - margin); // X 轴
    painter.drawLine(margin, margin, margin, height() - margin);                         // Y 轴

    // 绘制 Y 轴刻度和标签（分为 5 段）
    for (int i = 0; i <= 5; ++i) {
        int yPos = height() - margin - (chartHeight * i / 5); // 计算 Y 坐标位置
        painter.drawLine(margin - 5, yPos, margin, yPos);     // 刻度线
        QString label = QString::number(maxProb_ * i / 5, 'f', 2); // 刻度值
        painter.drawText(margin - 40, yPos + 4, label);       // 显示文本
    }

    // 添加 X/Y 的文字提示
    painter.drawText(margin - 35, height() - margin + 32, "X:");
    painter.drawText(margin - 35, height() - margin + 44, "Y:");

    QFontMetrics fm(painter.font()); // 获取当前字体度量信息

    // 遍历所有数据项，绘制柱子及其标签和相关坐标点
    for (size_t i = 0; i < data_.size(); ++i) {
        double prob = data_[i]; // 当前柱子的高度（概率值）

        // 计算柱子左上角 X 坐标
        int x = margin + i * (barWidth + spacing);

        if (std::isnan(prob)) {
            // 如果是 NaN，则画一个灰色的小矩形表示无效数据
            painter.setBrush(Qt::gray);
            painter.drawRect(x, height() - margin - 10, barWidth, 10);
        } else {
            // 否则根据概率比例计算柱子高度并绘制
            int y = height() - margin - static_cast<int>((prob / maxProb_) * chartHeight);
            painter.setBrush(getColorForLabel(labels_[i])); // 根据标签获取对应颜色
            painter.drawRect(x, y, barWidth, height() - margin - y); // 绘制柱子
        }

        // 绘制柱子的标签（如 P0, P1...）
        QString labelText = QString("P%1").arg(i);
        painter.drawText(x, height() - margin + 15, labelText);

        // 如果有对应的数据点，绘制其 X 和 Y 值
        if (i < points_.size()) {
            QPointF point = points_[i];
            QString xpointLabel = QString("%1").arg(point.x(), 0, 'f', 2); // X 值格式化
            int xtextWidth = fm.horizontalAdvance(xpointLabel); // 文字宽度
            painter.drawText(x + (barWidth - xtextWidth) / 2, height() - margin + 32, xpointLabel);

            QString ypointLabel = QString("%1").arg(point.y(), 0, 'f', 2); // Y 值格式化
            int ytextWidth = fm.horizontalAdvance(ypointLabel); // 文字宽度
            painter.drawText(x + (barWidth - ytextWidth) / 2, height() - margin + 44, ypointLabel);
        }
    }
}