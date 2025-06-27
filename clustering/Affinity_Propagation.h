#ifndef AFFINITY_PROPAGATION_H
#define AFFINITY_PROPAGATION_H

#include <iostream>
#include <vector>
#include <Eigen/Dense>           // Eigen库用于矩阵和向量运算
#include <Eigen/StdVector>
#include <algorithm>            // 提供排序、随机打乱等功能
#include <random>               // 用于更安全的随机数生成器

// 定义一个特殊值表示使用中位数作为Preference（偏好值）
#define MEDIAN -114514

class AffinityPropagation {
private:
    double Damping;              // 阻尼系数，防止震荡，取值在[0.5, 1)之间
    double Tol;                  // 收敛阈值，当责任和可用性变化小于该值时停止迭代
    double Preference;           // 偏好值，决定聚类中心数量的先验，默认为MEDIAN
    int Maxiter;                 // 最大迭代次数
    Eigen::MatrixXd X;           // 输入数据矩阵，每一行是一个样本点
    Eigen::MatrixXd Responsibility;     // 责任矩阵（responsibility matrix）
    Eigen::MatrixXd Availability;       // 可用性矩阵（availability matrix）
    Eigen::MatrixXd new_Responsibility; // 本次迭代更新后的责任矩阵
    Eigen::MatrixXd new_Availability;   // 本次迭代更新后的可用性矩阵

public:
    std::vector<int> indexcenters;      // 聚类中心的索引列表
    std::vector<std::vector<std::vector<double>>> center_history; // 每次迭代后中心的历史记录
    std::vector<std::vector<double>> centers;  // 当前确定的聚类中心坐标
    std::vector<int> labels;            // 每个样本点的聚类标签
    std::vector<std::vector<int>> label_history; // 标签历史记录，用于可视化或调试

    /**
     * 构造函数
     * @param damping 阻尼系数
     * @param tol 收敛阈值
     * @param x 数据集（每行一个样本）
     * @param preference 偏好值，默认为MEDIAN
     * @param maxiter 最大迭代次数，默认1000
     */
    AffinityPropagation(double damping, double tol, Eigen::MatrixXd x, double preference = MEDIAN, int maxiter = 1000)
        : Damping(damping), Tol(tol), X(x), Preference(preference), Maxiter(maxiter) {
        // 初始化责任矩阵和可用性矩阵为零矩阵
        Responsibility = Eigen::MatrixXd::Zero(X.rows(), X.rows());
        Availability = Eigen::MatrixXd::Zero(X.rows(), X.rows());
        new_Responsibility = Eigen::MatrixXd::Zero(X.rows(), X.rows());
        new_Availability = Eigen::MatrixXd::Zero(X.rows(), X.rows());
    }

    /**
     * 计算相似度矩阵（负欧氏距离平方）
     * @return 相似度矩阵 S，S(i,j) 表示点j对点i的吸引力
     */
    Eigen::MatrixXd distance() {
        Eigen::VectorXd row_norms = X.rowwise().squaredNorm(); // 每行的平方范数
        Eigen::MatrixXd dot_products = X * X.transpose();     // 内积矩阵
        Eigen::MatrixXd dists = (-2 * dot_products).rowwise() + row_norms.transpose(); // 距离计算
        dists = dists.colwise() + row_norms;
        return -dists; // 返回负距离作为相似度
    }

    /**
     * 将相似度矩阵的对角线设置为非对角元素的中位数
     * @param similarity 相似度矩阵
     */
    void setPreferenceToMedian(Eigen::MatrixXd& similarity) {
        int n = similarity.rows();
        Eigen::VectorXd off_diag(n * (n - 1));

        // 提取所有非对角线元素
        int idx = 0;
        for (int i = 0; i < n; ++i) {
            double* row_data = similarity.row(i).data();
            for (int j = 0; j < n; ++j) {
                if (i != j) {
                    off_diag(idx++) = row_data[j];
                }
            }
        }

        // 排序找中位数
        std::sort(off_diag.data(), off_diag.data() + off_diag.size());
        double median = off_diag[off_diag.size() / 2];

        // 设置对角线为中位数
        similarity.diagonal().setConstant(median);
    }

    /**
     * 设置偏好值到相似度矩阵的对角线上
     * @param similarity 相似度矩阵
     */
    void setPreference(Eigen::MatrixXd& similarity) {
        similarity.diagonal().setConstant(Preference);
    }

    /**
     * 更新责任矩阵 R(i,k) = s(i,k) - max_{k'≠k} [s(i,k') + a(i,k')]
     * @param similarity 相似度矩阵
     */
    void new_res(Eigen::MatrixXd& similarity) {
        Eigen::MatrixXd S_plus_A = similarity + Availability;

        #pragma omp parallel for
        for (int i = 0; i < S_plus_A.rows(); ++i) {
            Eigen::VectorXd row = S_plus_A.row(i);

            for (int k = 0; k < S_plus_A.cols(); ++k) {
                double temp = row(k);
                row(k) = -1e9; // 屏蔽当前k'
                double max_val = row.maxCoeff();
                new_Responsibility(i, k) = similarity(i, k) - max_val;
                row(k) = temp;  
            }
        }
    }

    /**
     * 更新可用性矩阵：
     * a(i,k) = min(0, r(k,k) + sum_{i'≠k,i'} max(0, r(i',k)))
     */
    void new_avai() {
        int n = Responsibility.rows();
        new_Availability = Eigen::MatrixXd::Zero(n, n);

        Eigen::MatrixXd posResponsibility = Responsibility.array().max(0.0); // 所有r(i,k)取max(0, r(i,k))
        Eigen::VectorXd diag_sums = posResponsibility.colwise().sum(); // 每列总和

        // 减去每个k对应的r(k,k)，得到除k外的其他点贡献的总和
        for (int k = 0; k < n; ++k) {
            diag_sums(k) -= std::max(0.0, Responsibility(k, k));
        }

        new_Availability.diagonal() = diag_sums;

        #pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            for (int k = 0; k < n; ++k) {
                if (i == k) continue;
                double sum_pos = diag_sums(k) - posResponsibility(i, k);
                new_Availability(i, k) = std::min(0.0, Responsibility(k, k) + sum_pos);
            }
        }
    }

    /**
     * 主循环：迭代更新责任和可用性矩阵直到收敛
     * @param similarity 相似度矩阵
     */
    void update(Eigen::MatrixXd similarity){
        int i = 0;
        while(i < Maxiter){
            new_res(similarity);
            double r_diff = (new_Responsibility - Responsibility).array().abs().maxCoeff();
            Responsibility = Damping * Responsibility + (1 - Damping) * new_Responsibility;

            new_avai();
            double a_diff = (new_Availability - Availability).array().abs().maxCoeff();
            Availability = Damping * Availability + (1 - Damping) * new_Availability;

            pick_center();
            center_history.push_back(centers);
            label_history.push_back(Assign_Labels());

            // 判断是否收敛
            if (r_diff < Tol && a_diff < Tol) {
                std::cout << "Converged at iteration " << i << std::endl;
                break;
            }
            i++;
        }
    }

    /**
     * 确定最终聚类中心
     */
    void pick_center(){
        Eigen::MatrixXd RA = Responsibility + Availability;
        std::vector<std::vector<double>> c;
        for(int i=0; i<RA.rows(); i++){
            if(RA(i, i) > 0) {
                bool is_exemplar = false;
                for(int j=0; j<RA.cols(); j++) {
                    int index;
                    double max_val = RA.row(j).maxCoeff(&index);
                    if(index == i) {
                        is_exemplar = true;
                        break;
                    }
                }
                if(is_exemplar) {
                    indexcenters.push_back(i);
                    std::vector<double> point = {X(i, 0), X(i, 1)};
                    c.push_back(point);
                }
            }
        }
        centers = c;
    }

    /**
     * 为每个样本分配最近的聚类中心标签
     * @return 标签数组
     */
    std::vector<int> Assign_Labels() {
        if (indexcenters.empty()) {
            return {};
        }

        std::vector<int> labels(X.rows());
        Eigen::MatrixXd RA = Responsibility + Availability;

        for (int i = 0; i < X.rows(); ++i) {
            int best_center_index = -1;
            double best_score = -std::numeric_limits<double>::infinity();

            for (size_t j = 0; j < indexcenters.size(); ++j) {
                int center_idx = indexcenters[j];
                double score = RA(i, center_idx);
                if (score > best_score) {
                    best_score = score;
                    best_center_index = center_idx;
                }
            }

            labels[i] = best_center_index;
        }

        return labels;
    }

    /**
     * 启动整个 Affinity Propagation 流程
     */
    void start(){
        Eigen::MatrixXd similarity = distance();
        if(Preference == MEDIAN){
            setPreferenceToMedian(similarity);
        }
        else{
            setPreference(similarity);
        }
        label_history.clear();
        center_history.clear();
        update(similarity);
        pick_center();
        labels = Assign_Labels();
    }

};

#endif // AFFINITY_PROPAGATION_H