#ifndef CONVERTION_H
#define CONVERTION_H

// 标准库头文件
#include <vector>
#include <iostream>

// Qt 头文件
#include <QList>     // 使用 QList 容器
#include <QPointF>   // 使用 QPointF 表示二维点 (x, y)

// Eigen 线性代数库
#include <Eigen/Dense>

/**
 * @brief 将 QList<QPointF> 转换为 Eigen::MatrixXd 矩阵
 *
 * 每个 QPointF 表示一个二维点 (x, y)，转换后矩阵的每一行对应一个点：
 * [x1, y1]
 * [x2, y2]
 * ...
 *
 * @param pointList 输入的 QPointF 列表
 * @return 转换后的 Eigen 矩阵（n × 2）
 */
Eigen::MatrixXd fromQPointListToEigen(const QList<QPointF>& pointList) {
    int nPoints = pointList.size();
    if (nPoints == 0) return Eigen::MatrixXd(); // 如果为空，返回空矩阵

    // 创建一个 nPoints x 2 的矩阵用于存储坐标
    Eigen::MatrixXd mat(nPoints, 2);

    for (int i = 0; i < nPoints; ++i) {
        const QPointF& pt = pointList[i];
        mat(i, 0) = pt.x(); // 填充 x 坐标
        mat(i, 1) = pt.y(); // 填充 y 坐标
    }

    return mat;
}

/**
 * @brief 将 Eigen::MatrixXd 转换为 QList<QPointF>
 *
 * 假设输入矩阵是 n × 2 的形式，每行表示一个二维点 (x, y)
 *
 * @param mat 输入的 Eigen 矩阵（n × 2）
 * @return 转换后的 QPointF 列表
 */
QList<QPointF> fromEigenToQPointList(const Eigen::MatrixXd& mat) {
    QList<QPointF> pointList;

    int rows = mat.rows();
    for (int i = 0; i < rows; ++i) {
        double x = mat(i, 0); // 获取 x 坐标
        double y = mat(i, 1); // 获取 y 坐标
        pointList.append(QPointF(x, y)); // 添加到列表
    }

    return pointList;
}

#endif // CONVERTION_H