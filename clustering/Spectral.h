#ifndef SPECTRAL_H
#define SPECTRAL_H

#include <iostream>
#include <vector>
#include <Eigen/Dense>           // Eigen 线性代数库
#include <Eigen/StdVector>
#include <Eigen/Eigenvalues>     // 用于特征值分解
#include <algorithm>            // 提供 shuffle 等算法
#include <random>               // 用于随机初始化
#include "K_Means.h"            // 引入 K-Means 聚类类

// 归一化方式枚举类型
enum Norm {
    NoNorm,   // 不归一化（原始拉普拉斯矩阵）
    RW,       // 随机游走归一化（Random Walk normalized）
    SYM       // 对称归一化（Symmetric normalized）
};

/**
 * Spectral：实现谱聚类算法（Spectral Clustering）
 */
class Spectral {
private:
    int K;              // 聚类数量
    double Sigma;       // RBF 核宽度参数
    Eigen::MatrixXd X;  // 输入数据集（每行一个样本）
    Norm Normstyle;     // 拉普拉斯矩阵归一化方式

public:
    std::vector<int> labels;                // 最终聚类标签
    std::vector<std::vector<int>> label_history; // 标签历史记录（来自 KMeans）

    /**
     * 构造函数
     * @param k 要聚类的数量
     * @param x 输入数据矩阵（每行一个样本）
     * @param normstyle 使用哪种归一化方式（默认为 NoNorm）
     * @param sigma RBF 核宽度参数（默认为1.0）
     */
    Spectral(int k, Eigen::MatrixXd x, Norm normstyle = NoNorm, double sigma = 1.0)
        : K(k), X(x), Normstyle(normstyle), Sigma(sigma) {}

    /**
     * 计算所有点之间的欧氏距离矩阵
     * @return 距离矩阵（N × N）
     */
    Eigen::MatrixXd distance() {
        Eigen::VectorXd row_norms = X.rowwise().squaredNorm(); // 每个样本的平方L2范数
        Eigen::MatrixXd dot_products = X * X.transpose();      // 点积矩阵
        Eigen::MatrixXd dists = (-2 * dot_products).rowwise() + row_norms.transpose();
        dists = dists.colwise() + row_norms;
        return dists.array().sqrt(); // 返回欧氏距离矩阵
    }

    /**
     * 构建相似度矩阵 W（使用 RBF 核）
     * @param sigma RBF 核宽度参数
     * @return 相似度矩阵 W
     */
    Eigen::MatrixXd getW(double sigma = 1.0) {
        Eigen::MatrixXd dists = distance();

        Eigen::MatrixXd W = (-dists / (2 * sigma * sigma)).array().exp(); // RBF核计算
        W.diagonal().setZero(); // 自环边置零
        return W;
    }

    /**
     * 构建度矩阵 D（对角线元素是 W 的行和）
     * @param W 相似度矩阵
     * @return 度矩阵 D
     */
    Eigen::MatrixXd getD(Eigen::MatrixXd W) {
        Eigen::VectorXd degree = W.rowwise().sum(); // 每行求和
        return degree.asDiagonal();                 // 转换为对角矩阵
    }

    /**
     * 构建原始拉普拉斯矩阵 L = D - W
     * @param W 相似度矩阵
     * @param D 度矩阵
     * @return 拉普拉斯矩阵 L
     */
    Eigen::MatrixXd getL(Eigen::MatrixXd W, Eigen::MatrixXd D) {
        return D - W;
    }

    /**
     * 构建对称归一化拉普拉斯矩阵 L_sym = I - D^(-1/2) * W * D^(-1/2)
     * @param W 相似度矩阵
     * @param D 度矩阵
     * @return 对称归一化的拉普拉斯矩阵
     */
    Eigen::MatrixXd getL_sym(Eigen::MatrixXd W, Eigen::MatrixXd D) {
        Eigen::MatrixXd D_inv_sqrt = D;
        for (int i = 0; i < D.rows(); ++i) {
            double diag_val = D(i, i);
            if (diag_val > 0) {
                D_inv_sqrt(i, i) = 1.0 / std::sqrt(diag_val); // D^(-1/2)
            } else {
                D_inv_sqrt(i, i) = 0;
            }
        }

        return Eigen::MatrixXd::Identity(D.rows(), D.cols()) - D_inv_sqrt * W * D_inv_sqrt;
    }

    /**
     * 构建随机游走归一化拉普拉斯矩阵 L_rw = I - D^(-1) * W
     * @param W 相似度矩阵
     * @param D 度矩阵
     * @return 随机游走归一化的拉普拉斯矩阵
     */
    Eigen::MatrixXd getL_rw(Eigen::MatrixXd W, Eigen::MatrixXd D) {
        Eigen::MatrixXd D_inv = D;
        for (int i = 0; i < D.rows(); ++i) {
            double diag_val = D(i, i);
            if (diag_val > 0) {
                D_inv(i, i) = 1.0 / diag_val; // D^(-1)
            } else {
                D_inv(i, i) = 0;
            }
        }

        return Eigen::MatrixXd::Identity(D.rows(), D.cols()) - D_inv * W;
    }

    /**
     * 获取前 K 个最小非零特征值对应的特征向量
     * @param L 拉普拉斯矩阵
     * @return 特征向量组成的矩阵 U（K 列）
     */
    Eigen::MatrixXd getEigen(Eigen::MatrixXd L) {
        // 求解特征值和特征向量（因为 L 是对称的，使用 SelfAdjointEigenSolver）
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> es(L);

        // 安全处理 K 的取值范围
        int max_possible_k = L.cols() - 1;  // 跳过第一个特征向量（对应特征值 0）
        int effective_k = std::min(K, max_possible_k);
        effective_k = std::max(1, effective_k);  // 至少取1个特征向量

        // 检查是否足够特征向量可用
        if (es.eigenvectors().cols() < effective_k + 1) {
            throw std::runtime_error("Not enough eigenvectors for K=" + std::to_string(K));
        }

        // 提取前 K 个最小非零特征向量（跳过第一个）
        Eigen::MatrixXd U = es.eigenvectors().block(0, 1, L.rows(), effective_k);
        return U;
    }

    /**
     * 启动整个谱聚类流程：
     * 1. 构建相似度矩阵 W
     * 2. 构建度矩阵 D 和拉普拉斯矩阵 L
     * 3. 提取特征向量
     * 4. 在特征空间上应用 K-Means 聚类
     */
    void start() {
        Eigen::MatrixXd W = getW(Sigma);         // 构建相似度矩阵
        Eigen::MatrixXd D = getD(W);             // 构建度矩阵
        Eigen::MatrixXd L;

        // 根据选择的归一化方式构建拉普拉斯矩阵
        if (Normstyle == RW) {
            L = getL_rw(W, D);
        } else if (Normstyle == SYM) {
            L = getL_sym(W, D);
        } else {
            L = getL(W, D);
        }

        // 提取前 K 个最小非零特征值对应的特征向量
        Eigen::MatrixXd U = getEigen(L);

        // 在特征空间上运行 K-Means 聚类
        K_Means k_means = K_Means(K, U, 30, 1e-4);
        k_means.start();

        // 保存结果
        labels = k_means.labels;
        label_history = k_means.label_history;
    }
};

#endif // SPECTRAL_H