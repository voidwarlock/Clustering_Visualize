#include "ShowProbs.h"
#include <QPainter>
#include <QFontMetrics>
#include <algorithm>
#include <iostream>
#include <vector>

SubWindowProbs::SubWindowProbs(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Probability Chart");

    scrollArea_ = new QScrollArea(this);
    chartWidget_ = new BarChartWidget(scrollArea_);
    scrollArea_->setWidget(chartWidget_);
    scrollArea_->setWidgetResizable(false); // 不自动缩放内部 widget
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(scrollArea_);
    setLayout(layout);
}

void SubWindowProbs::setData(const std::vector<double> &probs, const QList<QPointF> &points, const std::vector<int> &labels) {
    chartWidget_->setData(probs, points, labels);
    chartWidget_->resize((16 + 22) * probs.size(), chartWidget_->height()); // 动态设置宽度
}