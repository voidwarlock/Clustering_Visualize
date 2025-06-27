#ifndef K_MEANS_H
#define K_MEANS_H

#include <iostream>
#include <vector>
#include <Eigen/Dense>           // 用于矩阵运算
#include <Eigen/StdVector>
#include <algorithm>            // 提供 shuffle 等函数
#include <random>               // 用于随机数生成

/**
 * K_Means：实现经典的 K-Means 聚类算法（基于迭代优化）
 */
class K_Means {
private:
    int K;                      // 要聚类的数量（簇数）
    int Maxiter;                // 最大迭代次数
    double tol;                 // 收敛阈值（中心变化小于该值则停止）
    Eigen::MatrixXd X;          // 输入数据集（每行一个样本）
    Eigen::MatrixXd Center;     // 聚类中心矩阵（K × D）

public:
    std::vector<int> labels;                    // 每个样本对应的聚类标签
    std::vector<std::vector<int>> label_history; // 标签历史记录（可用于可视化或调试）
    std::vector<std::vector<double>> centers;   // 聚类中心（每次迭代后的结果）
    std::vector<std::vector<std::vector<double>>> center_history; // 中心历史记录

    /**
     * 构造函数（无参数构造函数）
     * @param x 输入数据矩阵（每行一个样本）
     */
    K_Means(Eigen::MatrixXd x) : X(x) {}

    /**
     * 构造函数（带参数构造函数）
     * @param k 聚类数量
     * @param x 输入数据矩阵
     * @param maxiter 最大迭代次数（默认为20）
     * @param tor 收敛容忍度（默认为1e-6）
     */
    K_Means(int k, Eigen::MatrixXd x, int maxiter = 20, double tor = 1e-6)
        : K(k), X(x), Maxiter(maxiter), tol(tor) {
        Center = Eigen::MatrixXd(K, x.cols()); // 初始化中心矩阵
        labels = std::vector<int>(x.rows());    // 初始化标签
        Init();                                 // 初始化聚类中心
    }

    /**
     * 初始化聚类中心：从数据中随机选取 K 个点作为初始中心
     */
    void Init() {
        std::vector<int> indices(X.rows());
        for (int i = 0; i < X.rows(); ++i) {
            indices[i] = i;
        }

        // 使用随机引擎打乱索引
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(indices.begin(), indices.end(), gen);

        // 选取前 K 个点作为初始中心
        for (int i = 0; i < K; ++i) {
            Center.row(i) = X.row(indices[i]);
        }
    }

    /**
     * 计算每个样本到各个聚类中心的欧氏距离
     * @return 距离矩阵（N × K），每行表示样本到各中心的距离
     */
    Eigen::MatrixXd distance() {
        // 计算输入样本的平方 L2 范数（每行一个样本）
        Eigen::VectorXd X_norms = X.rowwise().squaredNorm();

        // 计算聚类中心的平方 L2 范数（每行一个中心）
        Eigen::VectorXd C_norms = Center.rowwise().squaredNorm();

        // 计算点积矩阵：X * Center^T （N × K）
        Eigen::MatrixXd dot_products = X * Center.transpose();

        // 利用公式计算距离平方：||x_i - c_j||^2 = ||x_i||^2 + ||c_j||^2 - 2*x_i*c_j^T
        Eigen::MatrixXd dists_sq = (-2 * dot_products).rowwise() + C_norms.transpose();
        dists_sq = dists_sq.colwise() + X_norms;

        // 返回开根号后的欧氏距离矩阵
        return dists_sq.cwiseMax(0).array().sqrt();
    }

    /**
     * 更新聚类标签与中心
     * @param dists 各样本到各聚类中心的距离矩阵
     * @return 是否收敛（即中心变化小于 tol）
     */
    bool update(Eigen::MatrixXd dists) {
        Eigen::MatrixXd NewCenter = Eigen::MatrixXd(K, X.cols());
        std::vector<int> counts(K, 0); 

        NewCenter.setZero(); 

        // 分配每个样本到最近的聚类中心
        for (int i = 0; i < X.rows(); ++i) {
            int index;
            dists.row(i).minCoeff(&index); // 找到最近的中心索引
            labels[i] = index;             // 分配标签
            NewCenter.row(index) += X.row(i);
            counts[index]++;
        }

        // 更新每个聚类中心（取平均）
        for (int k = 0; k < K; ++k) {
            if (counts[k] > 0) { 
                NewCenter.row(k) /= static_cast<double>(counts[k]);
            }
        }

        // 判断是否收敛
        if (((NewCenter - Center).cwiseAbs().array() < tol).all()) {
            return true;
        } else {
            Center = NewCenter; // 更新中心
            return false;
        }
    }

    /**
     * 计算当前聚类的总成本（所有样本到其聚类中心的平方距离之和）
     * @return 当前成本值
     */
    double computeCost() {
        double cost = 0.0;
        for (int i = 0; i < X.rows(); ++i) {
            int label = labels[i];
            double dist = (X.row(i) - Center.row(label)).norm();
            cost += dist * dist;  
        }
        return cost;
    }

    /**
     * 将当前聚类中心转换为标准 vector<vector<double>> 格式
     */
    void get_center() {
        centers.clear();
        for (int k = 0; k < Center.rows(); ++k) {
            std::vector<double> center_point;
            for (int j = 0; j < Center.cols(); ++j) {
                center_point.push_back(Center(k, j));
            }
            centers.push_back(center_point);
        }
    }

    /**
     * 启动整个 K-Means 聚类流程
     */
    void start() {
        int i = 0;
        while (i < Maxiter) {
            if (update(distance())) {
                break; // 收敛则提前结束
            }

            label_history.push_back(labels); // 保存当前标签状态
            get_center();                    // 获取当前中心
            center_history.push_back(centers); // 保存中心历史


            i++;
        }
    }

};

#endif // K_MEANS_H