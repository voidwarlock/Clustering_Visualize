#ifndef SHOWTREE_H
#define SHOWTREE_H


#include <QDialog>
#include <QVBoxLayout>
#include <vector>
#include <QScrollArea>
#include "TreeChartWidget.h"

class SubWindowTree : public QDialog {
    Q_OBJECT

public:
    explicit SubWindowTree(QWidget *parent = nullptr);
    void setData(const std::vector<ClusterNode*> &roots, const QList<QPointF> &points, const std::vector<int> &labels, int nclusters);

private:
    QScrollArea *scrollArea_;
    TreeChartWidget *chartWidget_;
};

#endif