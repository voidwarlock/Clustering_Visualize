#ifndef AGGLOMERATIVE_H
#define AGGLOMERATIVE_H

#include <iostream>
#include <vector>
#include <Eigen/Dense>           // 用于矩阵运算
#include <Eigen/StdVector>
#include <algorithm>            // 提供排序等功能
#include <random>               // 用于随机数生成
#include <queue>                // 使用优先队列实现最小堆

/**
 * ClusterNode：表示聚类树中的一个节点
 */
struct ClusterNode {
    int id;                      // 节点唯一标识符（初始为数据点索引）
    int height;                  // 树的高度，用于可视化或层次分析
    std::vector<int> ids;        // 当前簇包含的所有原始数据点索引
    ClusterNode* left;           // 左子节点
    ClusterNode* right;          // 右子节点

    /**
     * 构造函数
     * @param _id 节点ID
     * @param _height 当前高度
     * @param _left 左子节点
     * @param _right 右子节点
     */
    ClusterNode(int _id, int _height = 0, ClusterNode* _left = nullptr, ClusterNode* _right = nullptr)
        : id(_id), height(_height), left(_left), right(_right) {
        ids.push_back(id);       // 初始化时只包含自己
    }
};

/**
 * ClusterPair：用于优先队列中存储两个簇之间的距离信息
 */
struct ClusterPair {
    ClusterNode* node1;         // 第一个簇节点
    ClusterNode* node2;         // 第二个簇节点
    double distance;            // 两簇之间的距离

    // 重载小于号，使priority_queue成为最小堆（按距离从小到大出队）
    bool operator<(const ClusterPair& other) const {
        return distance > other.distance; 
    }
};

/**
 * Agglomerative：自底向上的层次聚类类（Hierarchical Clustering）
 */
class Agglomerative {
private:
    std::vector<ClusterNode*> nodes; // 所有聚类节点集合
    Eigen::MatrixXd X;               // 输入数据集，每行是一个样本点
    Eigen::MatrixXd dists;           // 样本点之间的成对欧氏距离矩阵
    std::priority_queue<ClusterPair> PossibleClusters; // 优先队列，保存当前所有可能合并的簇对
    std::vector<bool> is_valid;      // 标记每个节点是否仍然有效（未被合并）
    int Numclusters;                 // 用户指定的目标聚类数量

public:
    std::vector<int> labels;         // 最终聚类标签数组（每个样本对应簇编号）
    std::vector<ClusterNode*> roots; // 当前所有根节点（代表当前簇）
    std::vector<std::vector<int>> label_history; // 每次迭代后的标签历史记录
    std::vector<std::vector<ClusterNode*>> root_history; // 每次迭代后的根节点历史记录
    std::vector<int> num_history;    // 每次迭代后剩余簇的数量变化记录

    /**
     * 构造函数
     * @param x 数据集（每行一个样本）
     * @param numclusters 目标聚类数
     */
    Agglomerative(Eigen::MatrixXd x, int numclusters)
        : X(x), Numclusters(numclusters) {
        is_valid = std::vector<bool>(x.rows(), true);
        for (int i = 0; i < x.rows(); ++i) {
            ClusterNode* node = new ClusterNode(i); // 初始每个点都是独立簇
            nodes.push_back(node);
            roots.push_back(node);
        }
    }

    /**
     * 计算样本之间的欧氏距离矩阵
     */
    void distance() {
        Eigen::VectorXd row_norms = X.rowwise().squaredNorm();
        Eigen::MatrixXd dot_products = X * X.transpose();
        dists = (-2 * dot_products).rowwise() + row_norms.transpose();
        dists = dists.colwise() + row_norms;
        dists = dists.array().sqrt(); // 开平方得到欧氏距离
    }

    /**
     * 初始化优先队列，将所有初始簇对加入队列
     */
    void init_PossibleCLusters() {
        for (int i = 0; i < dists.rows(); ++i) {
            for (int j = i + 1; j < dists.cols(); ++j) {
                PossibleClusters.push({nodes[i], nodes[j], dists(i, j)});
            }
        }
    }

    /**
     * 使用平均链接法（Average Linkage）计算两个簇之间的距离
     * @param node1 簇1
     * @param node2 簇2
     * @return 两簇之间的平均距离
     */
    double update_average_linkage_distance(ClusterNode* node1, ClusterNode* node2) {
        int count = 0;
        double total = 0.0;

        #pragma omp parallel for reduction(+:total,count)
        for (int i : node1->ids) {
            for (int j : node2->ids) {
                total += dists(i, j);
                ++count;
            }
        }

        return count > 0 ? total / count : std::numeric_limits<double>::infinity();
    }

    /**
     * 为每个样本分配当前聚类标签
     * @return 标签数组
     */
    std::vector<int> Assign_Labels() const {
        std::vector<int> labels(X.rows());

        int label = 0;
        for (const ClusterNode* node : nodes) {
            if (is_valid[node->id]) {
                for (int idx : node->ids) {
                    if (idx < X.rows()) { // 只对原始数据点赋值
                        labels[idx] = label;
                    }
                }
                label++;
            }
        }

        return labels;
    }

    /**
     * 层次聚类主循环：不断合并最近的簇直到达到目标簇数
     */
    void update() {
        int currentid = X.rows(); // 新簇的起始ID
        int N = X.rows();         // 当前簇数

        while (!PossibleClusters.empty()) {
            ClusterPair curr = PossibleClusters.top();
            PossibleClusters.pop();

            if (!is_valid[curr.node1->id] || !is_valid[curr.node2->id]) continue;

            // 从根列表中移除这两个节点
            roots.erase(std::remove(roots.begin(), roots.end(), curr.node1), roots.end());
            roots.erase(std::remove(roots.begin(), roots.end(), curr.node2), roots.end());

            // 标记这两个簇为无效
            is_valid[curr.node1->id] = false;
            is_valid[curr.node2->id] = false;

            // 创建新簇
            ClusterNode* new_node = new ClusterNode(currentid, 0, curr.node1, curr.node2);
            new_node->height = std::max(curr.node1->height, curr.node2->height) + 1;
            new_node->ids = curr.node1->ids;
            new_node->ids.insert(new_node->ids.end(), curr.node2->ids.begin(), curr.node2->ids.end());

            // 将新簇与现有簇的距离加入优先队列
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (is_valid[i] && nodes[i] != curr.node1 && nodes[i] != curr.node2) {
                    double avg_dist = update_average_linkage_distance(new_node, nodes[i]);
                    PossibleClusters.push({new_node, nodes[i], avg_dist});
                }
            }

            // 添加新簇到节点列表
            roots.push_back(new_node);
            nodes.push_back(new_node);
            is_valid.push_back(true);

            // 记录状态
            std::vector<int> temp_label = Assign_Labels();
            label_history.push_back(temp_label);
            root_history.push_back(roots);
            N -= 1;
            num_history.push_back(N);

            if (N == Numclusters) {
                labels = temp_label;
            }

            currentid++;
        }
    }

    /**
     * 启动整个层次聚类流程
     */
    void start() {
        distance();             // 计算距离矩阵
        init_PossibleCLusters(); // 初始化优先队列
        update();               // 进行聚类
    }

};

#endif // AGGLOMERATIVE_H