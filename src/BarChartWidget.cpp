// BarChartWidget.cpp
#include "BarChartWidget.h"
#include <QPainter>
#include <QFontMetrics>
#include "Label2Color.h"

BarChartWidget::BarChartWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(600); // 设置最小高度
}

void BarChartWidget::setData(const std::vector<double> &data, const QList<QPointF> &points, const std::vector<int> &labels) {
    data_ = data;
    points_ = points;
    labels_ = labels;
    if (!data_.empty()) {
        maxProb_ = *std::max_element(data_.begin(), data_.end());
    }
    update();
}

void BarChartWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int margin = 50;
    int barWidth = 16;      // 调整柱子宽度
    int spacing = 22;       // 调整柱子间距
    int chartHeight = height() - 2 * margin;
    int chartWidth = (barWidth + spacing) * data_.size();

    // 设置水平滚动区域大小（如果需要）
    setMinimumWidth(chartWidth + 2*margin); // 根据实际内容宽度设置最小宽度

    // 绘制背景
    painter.fillRect(rect(), Qt::white);

    // 绘制坐标轴
    painter.setPen(Qt::black);
    painter.drawLine(margin, height() - margin, margin + chartWidth, height() - margin);
    painter.drawLine(margin, margin, margin, height() - margin);

    // Y 轴刻度
    for (int i = 0; i <= 5; ++i) {
        int yPos = height() - margin - (chartHeight * i / 5);
        painter.drawLine(margin - 5, yPos, margin, yPos);
        QString label = QString::number(maxProb_ * i / 5, 'f', 2);
        painter.drawText(margin - 40, yPos + 4, label);
    }

    
    painter.drawText(margin - 35, height() - margin + 32, "X:");
    painter.drawText(margin - 35, height() - margin + 44, "Y:");

    QFontMetrics fm(painter.font());
    // 绘制柱子及其标签和points_
    
    for (size_t i = 0; i < data_.size(); ++i) {
        double prob = data_[i];
        int x = margin + i * (barWidth + spacing);
        
        if (std::isnan(prob)) {
            painter.setBrush(Qt::gray);
            painter.drawRect(x, height() - margin - 10, barWidth, 10);
        } else {
            int y = height() - margin - static_cast<int>((prob / maxProb_) * chartHeight);
            painter.setBrush(getColorForLabel(labels_[i]));
            painter.drawRect(x, y, barWidth, height() - margin - y);
        }
        
        QString labelText = QString("P%1").arg(i);
        painter.drawText(x, height() - margin + 15, labelText);

        // 绘制points_的数据，显示为 (x, y) 形式，并换行显示
        if (i < points_.size()) {
            QPointF point = points_[i];
            QString xpointLabel = QString("%1").arg(point.x(), 0, 'f', 2);
            int xtextWidth = fm.horizontalAdvance(xpointLabel);
            painter.drawText(x + (barWidth - xtextWidth) / 2, height() - margin + 32, xpointLabel);
            QString ypointLabel = QString("%1").arg(point.y(), 0, 'f', 2);
            int ytextWidth = fm.horizontalAdvance(ypointLabel);
            painter.drawText(x + (barWidth - ytextWidth) / 2, height() - margin + 44, ypointLabel);
            
        }
    }
}