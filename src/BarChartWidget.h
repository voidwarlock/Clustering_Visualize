#ifndef BARCHARTWIDGET_H
#define BARCHARTWIDGET_H


#pragma once
#include <QWidget>
#include <vector>
#include <QList>
#include <QPoint>

class BarChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit BarChartWidget(QWidget *parent = nullptr);
    void setData(const std::vector<double> &data, const QList<QPointF> &points, const std::vector<int> &labels);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<double> data_;
    QList<QPointF> points_;
    std::vector<int> labels_;
    double maxProb_ = 1.0;
};


#endif