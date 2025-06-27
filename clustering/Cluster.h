#ifndef CLUSTER_H
#define CLUSTER_H

// 包含各类聚类算法的头文件
#include "Affinity_Propagation.h"
#include "Agglomerative.h"
#include "DBSCAN.h"
#include "DPMM.h"
#include "K_Means.h"
#include "Spectral.h"

// 定义支持的聚类算法类型枚举
enum ClusterType {
    None,               // 无聚类
    k_means,            // K-Means 聚类
    dbscan,             // DBSCAN 密度聚类
    agglomerative,      // 层次聚类（自底向上）
    dpmm,               // 狄利克雷过程混合模型（非参数贝叶斯聚类）
    affinity_propagation, // 相似性传播聚类
    spectral            // 谱聚类
};

// 聚类参数结构体，用于统一配置不同聚类算法的参数
struct ClusteringParams {
    ClusterType clustertype;    // 指定使用的聚类算法类型
    int k;                      // K-Means 和谱聚类中簇的数量
    double eps;                 // DBSCAN 中邻域半径
    int minpts;                 // DBSCAN 中最小点数
    int nClusters;              // 层次聚类中的目标簇数量
    double alpha;               // DPMM 中浓度参数
    double damping;             // Affinity Propagation 中阻尼系数
    double preference;          // Affinity Propagation 中偏好值
    double tol;                 // 收敛容忍度（如 K-Means、AP）
    int maxiter;                // 最大迭代次数
    double sigma;               // 谱聚类中高斯核参数
    Norm normType;              // 谱聚类中归一化方式
    Inittype initType;          // K-Means 初始化方式
    int n_neighbors;            // 谱聚类或其它算法中最近邻数量
};

/**
 * ALLCluster：统一接口类，封装多种聚类算法的调用逻辑
 */
class ALLCluster {
public:
    Eigen::MatrixXd X;                        // 输入数据集（每行一个样本）

    ClusteringParams params;                  // 聚类参数配置

    std::vector<int> labels;                  // 最终聚类结果标签（每个样本对应一个簇编号）

    std::vector<std::vector<double>> centers; // 聚类中心（适用于 K-Means、AP 等）

    std::vector<Pointtype> point_features;    // 点特征（主要用于 DBSCAN 的核心/边界/噪声分类）

    std::vector<double> probs;                // 概率分布（用于 DPMM）

    std::vector<ClusterNode*> roots;          // 根节点列表（用于层次聚类构建树）

    // 历史记录字段（可用于可视化、调试、动画展示等）
    std::vector<std::vector<int>> label_history;              // 每次迭代后的标签变化
    std::vector<std::vector<std::vector<double>>> center_history; // 中心变化历史
    std::vector<std::vector<Pointtype>> point_feature_history;   // 点特征变化历史
    std::vector<std::vector<double>> prob_history;               // 概率分布变化历史
    std::vector<std::vector<ClusterNode*>> root_history;         // 树根节点变化历史
    std::vector<int> num_history;                                // 当前簇数变化历史

    /**
     * 构造函数
     * @param x 输入数据矩阵（每行一个样本）
     * @param Params 聚类参数配置
     */
    ALLCluster(Eigen::MatrixXd x, ClusteringParams Params)
        : X(x), params(Params) {}

    /**
     * 设置新的输入数据
     * @param x 新的数据矩阵
     */
    void setDatas(Eigen::MatrixXd x) {
        X = x;
    }

    /**
     * 设置新的聚类参数
     * @param Params 新的参数配置
     */
    void setParams(ClusteringParams Params) {
        params = Params;
    }

    /**
     * 启动聚类流程：根据参数选择并执行对应的聚类算法
     */
    void start() {
        // 清空之前的结果与历史记录
        labels.clear();
        centers.clear();
        point_features.clear();
        probs.clear();
        roots.clear();

        label_history.clear();
        center_history.clear();
        point_feature_history.clear();
        prob_history.clear();
        root_history.clear();
        num_history.clear();

        // 根据聚类类型选择具体算法并执行
        if (params.clustertype == k_means) {
            K_Means c = K_Means(params.k, X, params.maxiter, params.tol);
            c.start();
            labels = c.labels;
            centers = c.centers;

            label_history = c.label_history;
            center_history = c.center_history;
        }

        if (params.clustertype == dbscan) {
            DBSCAN c = DBSCAN(params.eps, params.minpts, X);
            c.start();
            labels = c.labels;
            point_features = c.point_features;

            label_history = c.label_history;
            point_feature_history = c.point_feature_history;
        }

        if (params.clustertype == agglomerative) {
            Agglomerative c = Agglomerative(X, params.nClusters);
            c.start();
            labels = c.labels;
            roots = c.roots;

            label_history = c.label_history;
            root_history = c.root_history;
            num_history = c.num_history;
        }

        if (params.clustertype == dpmm) {
            DPMM c = DPMM(params.alpha, X, params.maxiter);
            c.start();
            labels = c.Z;
            probs = c.Probs;

            label_history = c.label_history;
            prob_history = c.prob_history;
        }

        if (params.clustertype == affinity_propagation) {
            AffinityPropagation c = AffinityPropagation(params.damping, params.tol, X, params.preference, params.maxiter);
            c.start();
            labels = c.labels;
            centers = c.centers;

            label_history = c.label_history;
            center_history = c.center_history;
        }

        if (params.clustertype == spectral) {
            Spectral c = Spectral(params.k, X, params.normType);
            c.start();
            labels = c.labels;

            label_history = c.label_history;
        }
    }

};

#endif // CLUSTER_H