#ifndef COORDINATEWIDGET_H
#define COORDINATEWIDGET_H

#include <QWidget>
#include <QList>
#include <QPoint>
#include <QWheelEvent>
#include <QSlider>
#include <iostream>
#include <vector>
#include "clustering/Cluster.h"
#include "ShowProbs.h"
#include "ShowTree.h"

class CoordinateWidget : public QWidget {
    Q_OBJECT

public:
    explicit CoordinateWidget(QWidget *parent = nullptr);

    const QList<QPointF>& getPoints() const;
    void setPoints(const QList<QPointF> &points);
    void setScale(qreal scale);
    qreal getScale() const;

public slots:    
    void setLabels(const std::vector<int>& w_labels);
    void setCenters(const std::vector<std::vector<double>>& w_centers);
    void setPoint_features(const std::vector<Pointtype>& w_point_features, double w_eps);
    void setProbs(const std::vector<double>& w_probs);
    void setRoots(const std::vector<ClusterNode*>& w_roots, int nClusters);
    void setFlags(bool flag);

signals:
    void scaleChanged(qreal scale);
    void pointsChanged(const QList<QPointF> &points);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void onScaleSliderChanged(int value);

private:
    void drawGrid(QPainter *painter);
    void drawAxes(QPainter *painter);
    void drawCircle(QPainter *painter, const QPointF &point, int label);
    void drawCore(QPainter *painter, const QPointF &point, int label);
    void drawCross(QPainter *painter, const QPointF &point);
    void drawLegend(QPainter *painter);

    QList<QPointF> points;
    qreal scale = 1.0;
    QPointF offset;
    QPoint lastMousePosition;
    bool dragging = false;
    int Pointscale = 50;
    QSlider *scaleSlider = nullptr;

    SubWindowProbs *prob_window;
    SubWindowTree *tree_window;

    bool drawAuxi = false;
    std::vector<int> labels;
    std::vector<std::vector<double>> centers;
    std::vector<Pointtype> point_features;
    double eps;
};

#endif // COORDINATEWIDGET_H