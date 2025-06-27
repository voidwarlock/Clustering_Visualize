#ifndef DBSCAN_H
#define DBSCAN_H

#include <iostream>
#include <vector>
#include <Eigen/Dense>           // 用于矩阵运算
#include <Eigen/StdVector>
#include <algorithm>            // 提供排序等功能
#include <random>               // 用于随机数生成

/**
 * Pointtype：用于标识点的类型
 */
enum Pointtype {
    Corepoint,      // 核心点：在半径 Eps 内至少包含 Minpts 个邻居
    Marginpoint,    // 边界点：不是核心点，但属于某个簇的一部分
    Noisepoint      // 噪声点：不属于任何簇
};

/**
 * DBSCAN：基于密度的聚类算法（Density-Based Spatial Clustering of Applications with Noise）
 */
class DBSCAN {
private:
    double Eps;                     // 邻域半径（epsilon）
    int Minpts;                     // 成为核心点所需的最小邻域点数
    Eigen::MatrixXd X;              // 输入数据集（每行一个样本）

    std::vector<int> density;       // 每个点的密度（即其邻域内的点数量）
    std::vector<std::vector<int>> neighbors; // 每个点的邻居索引列表
    std::vector<bool> visited;      // 标记是否已访问该点

public:
    std::vector<Pointtype> point_features; // 点的类型信息（核心、边界、噪声）
    std::vector<int> labels;               // 聚类结果标签（-1 表示噪声）
    std::vector<std::vector<int>> label_history;          // 每次迭代后的标签历史记录
    std::vector<std::vector<Pointtype>> point_feature_history; // 每次迭代后的点类型历史记录

    /**
     * 构造函数
     * @param eps 邻域半径 epsilon
     * @param minpts 最小邻域点数
     * @param x 数据集（每行一个样本）
     */
    DBSCAN(double eps, int minpts, Eigen::MatrixXd x)
        : Eps(eps), Minpts(minpts), X(x) {
        labels = std::vector<int>(X.rows(), -1); // 初始化为 -1（未分类或噪声）
        point_features = std::vector<Pointtype>(X.rows());
        density = std::vector<int>(X.rows(), 0);
        neighbors = std::vector<std::vector<int>>(X.rows());
        visited = std::vector<bool>(X.rows(), false);
    }

    /**
     * 计算每个点的邻居及其密度（基于欧氏距离）
     */
    void distance() {
        int n = X.rows();                   // 数据点数量
        Eigen::VectorXd row_norms = X.rowwise().squaredNorm(); // 每个样本的平方L2范数

        // 计算所有点之间的平方距离：dists_sq(i,j) = ||x_i - x_j||^2
        Eigen::MatrixXd dot_products = X * X.transpose();
        Eigen::MatrixXd dists_sq = (-2 * dot_products).rowwise() + row_norms.transpose();
        dists_sq = dists_sq.colwise() + row_norms;

        double eps_sq = Eps * Eps; // 使用平方避免开根号计算

        // 构建邻居列表和密度
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != j && dists_sq(i, j) <= eps_sq) {
                    ++density[i];             // 邻居数量增加
                    neighbors[i].push_back(j); // 添加邻居索引
                }
            }

            // 根据密度确定点类型
            if (density[i] >= Minpts) {
                point_features[i] = Corepoint;
            } else if (density[i] > 0) {
                point_features[i] = Marginpoint;
            } else {
                point_features[i] = Noisepoint;
            }

            point_feature_history.push_back(point_features); // 保存状态快照
        }
    }

    /**
     * 扩展当前簇：将连通的核心点及其邻居加入同一簇
     * @param point 当前处理的点索引
     * @param label 当前簇的标签
     */
    void expandCluster(int point, int label) {
        std::queue<int> queue; // BFS队列
        queue.push(point);
        visited[point] = true;
        labels[point] = label; // 分配簇标签

        label_history.push_back(labels); // 保存当前标签状态
        point_feature_history.push_back(point_features);

        while (!queue.empty()) {
            int current = queue.front();
            queue.pop();

            // 如果是核心点，则继续扩展
            if (point_features[current] == Corepoint) {
                for (int neighbor : neighbors[current]) {
                    if (!visited[neighbor]) {
                        visited[neighbor] = true;
                        labels[neighbor] = label; // 分配相同簇标签

                        label_history.push_back(labels); // 保存状态
                        point_feature_history.push_back(point_features);

                        queue.push(neighbor);
                    }
                }
            }
        }
    }

    /**
     * 主逻辑：遍历所有点并进行聚类
     */
    void update() {
        int label = 0; // 当前簇编号
        for (int i = 0; i < X.rows(); ++i) {
            // 如果是核心点且未被访问过
            if (point_features[i] == Corepoint && !visited[i]) {
                labels[i] = label; // 设置簇标签
                visited[i] = true;
                expandCluster(i, label); // 扩展簇
                label++; // 下一个簇
            }
        }
    }

    /**
     * 启动整个 DBSCAN 聚类流程
     */
    void start() {
        distance();         // 第一步：计算每个点的邻居和密度
        update();           // 第二步：执行聚类
    }

};

#endif // DBSCAN_H