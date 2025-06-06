#ifndef CONVERTION_H
#define CONVERTION_H

#include <vector>
#include <QList>
#include <QPoint>
#include <Eigen/Dense>

Eigen::MatrixXd fromQPointListToEigen(const QList<QPointF>& pointList) {
    int nPoints = pointList.size();
    if (nPoints == 0) return Eigen::MatrixXd();

    // 假设每个点是二维的 (x, y)
    Eigen::MatrixXd mat(nPoints, 2);

    for (int i = 0; i < nPoints; ++i) {
        const QPointF& pt = pointList[i];
        mat(i, 0) = pt.x(); // x 坐标
        mat(i, 1) = pt.y(); // y 坐标
    }

    return mat;
}

QList<QPointF> fromEigenToQPointList(const Eigen::MatrixXd& mat) {
    QList<QPointF> pointList;

    int rows = mat.rows();
    for (int i = 0; i < rows; ++i) {
        double x = mat(i, 0);
        double y = mat(i, 1);
        pointList.append(QPointF(x, y));
    }

    return pointList;
}

#endif