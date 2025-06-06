#ifndef TREECHARTWIDGET_H
#define TREECHARTWIDGET_H

#pragma once
#include <QWidget>
#include <vector>
#include <QList>
#include <QPoint>
#include "clustering/Cluster.h"

#define MARGIN 50
#define PERHEIGHT 100 
#define PERWIDTH 45       

class TreeChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit TreeChartWidget(QWidget *parent = nullptr);
    void setData(const std::vector<ClusterNode*> &roots, const QList<QPointF> &points, const std::vector<int> &labels, int maxheight, int nclusters);
    void initColorBiases(int n_clusters);
    std::pair<int, int> drawNode(QPainter& painter, ClusterNode* node, int chartHeight);


protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int max_height;
    std::vector<ClusterNode*> roots_;
    QList<QPointF> points_;
    std::vector<int> labels_;
    std::vector<double> colorBiases_;
};


#endif