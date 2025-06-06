#ifndef SHOWPROBS_H
#define SHOWPROBS_H

// SubWindowProbs.h
#include <QDialog>
#include <QScrollArea>
#include <QVBoxLayout>
#include "BarChartWidget.h"


class SubWindowProbs : public QDialog {
    Q_OBJECT

public:
    explicit SubWindowProbs(QWidget *parent = nullptr);
    void setData(const std::vector<double> &probs, const QList<QPointF> &points, const std::vector<int> &labels);

private:
    QScrollArea *scrollArea_;
    BarChartWidget *chartWidget_;
};


#endif // SHOWPROBS_H