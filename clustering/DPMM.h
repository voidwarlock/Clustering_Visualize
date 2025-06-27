#ifndef DPMM_H
#define DPMM_H

#include <iostream>
#include <vector>
#include <Eigen/Dense>        // 用于矩阵运算
#include <Eigen/StdVector>    // 支持Eigen类型在STL容器中的使用
#include <algorithm>          // 用于std::random_shuffle等算法
#include <random>             // 随机数生成
#include "KNN.h"              // K近邻算法实现

// 初始化类型枚举
enum Inittype {SingleInit,   // 每个样本作为独立簇初始化
               KnnInit};     // 使用K近邻初始化

// Normal-Inverse-Wishart (NIW) 分布参数结构体
struct NiwParams{
    Eigen::VectorXd mu0;     // 先验均值向量
    double kappa0;           // 先验精度（均值置信度）
    int nu0;                 // 自由度（影响协方差矩阵）
    Eigen::MatrixXd Psi0;    // 缩放矩阵（协方差先验）

    // 构造函数
    NiwParams(int dim, double kappa, double nu, const Eigen::MatrixXd& psi)
        : mu0(Eigen::VectorXd::Zero(dim)), 
          kappa0(kappa), 
          nu0(nu), 
          Psi0(psi) {}
};

// 聚类簇类，表示DPMM中的一个聚类组件
class Cluster{
private:
    std::vector<Eigen::VectorXd> datas;  // 属于该簇的数据点
    int D;                              // 数据维度
    NiwParams niwparams;                // NIW先验参数

    // 簇统计量
    Eigen::VectorXd mean;               // 当前均值
    Eigen::MatrixXd covariance;         // 当前协方差矩阵
    
    Eigen::MatrixXd meanError;          // 均值误差矩阵
    int meanDf;                         // 均值自由度
    
    // 充分统计量
    Eigen::VectorXd sum;                // 数据和
    Eigen::MatrixXd sq_sum;             // 数据平方和矩阵
    
    // 缓存优化
    bool cache_flag;                    // 协方差矩阵缓存标志
    double cache_covariance_determinant;// 缓存的行列式值
    Eigen::MatrixXd cache_covariance_inverse; // 缓存的逆矩阵

public:
    int count;  // 簇中数据点数量

    // 默认构造函数（空簇）
    Cluster() : D(0), niwparams(NiwParams(0, 0.0, 0, Eigen::MatrixXd::Identity(0, 0))) {
        // 初始化所有成员为零/空
        mean = Eigen::VectorXd::Zero(0);
        covariance = Eigen::MatrixXd::Identity(0, 0);
        sum = Eigen::VectorXd::Zero(0);
        sq_sum = Eigen::MatrixXd::Zero(0, 0);
        meanError = Eigen::MatrixXd::Zero(0, 0);
        meanDf = 0;
        cache_flag = false;
        count = 0;
    }

    // 带参数的构造函数
    Cluster(int D, NiwParams niwParams) : D(D), niwparams(niwParams){
        // 初始化统计量
        mean = Eigen::VectorXd(D);
        covariance = Eigen::MatrixXd::Identity(D, D);
        sum = Eigen::VectorXd::Zero(D);
        sq_sum = Eigen::MatrixXd::Zero(D, D);
        
        // 计算初始均值误差和自由度
        meanError = calculateMeanError(niwParams.Psi0, niwParams.kappa0, niwParams.nu0);
        meanDf = std::max(0, niwParams.nu0 - D + 1);
    }

    // 计算均值误差矩阵
    Eigen::MatrixXd calculateMeanError(Eigen::MatrixXd Psi, int kappa, int nu){
        return Psi * (1.0 / (kappa * nu - D + 1.0));
    }

    // 添加数据点到簇
    void addData(Eigen::VectorXd data){
        count++;
        sum += data;                          // 更新数据和
        sq_sum += data * data.transpose();    // 更新平方和
        datas.push_back(data);                // 存储数据点
        updateParameters();                   // 更新簇参数
    }

    // 从簇中移除数据点
    void removeData(Eigen::VectorXd data){
        auto it = std::find(datas.begin(), datas.end(), data);
        if(it != datas.end()){
            int index = std::distance(datas.begin(), it);
            count--;
            sum -= data;                      // 更新数据和
            sq_sum -= data * data.transpose();// 更新平方和
            datas.erase(datas.begin() + index); // 移除数据点
            updateParameters();               // 更新簇参数
        }
    }

    // 更新簇参数（均值和协方差）
    void updateParameters(){
        int n = datas.size();
        if(n <= 0) return;
        
        // 更新NIW后验参数
        int kappa = niwparams.kappa0 + n;
        int nu = niwparams.nu0 + n;
        
        Eigen::VectorXd mu = sum / n;                     // 样本均值
        Eigen::VectorXd mu_mu = mu - niwparams.mu0;       // 均值差
        
        // 计算散布矩阵
        Eigen::MatrixXd C = sq_sum - ((mu * mu.transpose()) * n);
        
        // 更新Psi矩阵
        Eigen::MatrixXd psi = niwparams.Psi0 + C + 
                             ((mu_mu * mu_mu.transpose()) * (niwparams.kappa0 * n / (double)kappa));
        
        // 计算后验均值和协方差
        mean = (niwparams.mu0 * niwparams.kappa0 + mu * n) / (double)kappa;
        covariance = psi * ((kappa + 1.0) / (kappa * (nu - D + 1.0)));
        
        // 更新缓存标志
        cache_flag = false;
        
        // 重新计算均值误差和自由度
        meanError = calculateMeanError(psi, kappa, nu);
        meanDf = std::max(0, nu - D + 1);
    }

    // 计算数据点在该簇下的对数后验概率（学生t分布）
    double LogPosteriorPDF(Eigen::VectorXd data){
        Eigen::VectorXd x_mu = data - mean;
        
        // 缓存协方差矩阵的逆和行列式（如果未缓存）
        if(!cache_flag){
            Eigen::FullPivLU<Eigen::MatrixXd> lu(covariance);
            cache_covariance_determinant = lu.determinant();
            cache_covariance_inverse = lu.inverse();
            cache_flag = true;
        }
        
        // 计算马氏距离
        double x_muInvSx_muT = x_mu.transpose() * cache_covariance_inverse * x_mu;
        
        // 计算学生t分布的对数概率
        double log_prob = std::lgamma((meanDf + D)/2.0) 
                        - std::lgamma(meanDf/2.0) 
                        - (D/2.0) * std::log(meanDf * M_PI) 
                        - 0.5 * std::log(cache_covariance_determinant)
                        - ((meanDf + D)/2.0) * std::log(1.0 + x_muInvSx_muT / meanDf);
        return log_prob;
    }
};

// Dirichlet Process Mixture Model (DPMM) 类
class DPMM {
private:
    int D;                      // 数据维度
    int Maxiter;                // 最大迭代次数
    double Alpha;               // DP浓度参数
    Eigen::MatrixXd X;          // 数据矩阵（每行一个样本）
    NiwParams niwparams;        // NIW先验参数
    std::vector<Cluster> cluster_stats; // 所有簇的统计信息

public:
    std::vector<int> Z;                     // 簇分配结果
    std::vector<double> Probs;              // 每个样本的分配概率
    std::vector<std::vector<int>> label_history; // 簇分配历史（用于动画）
    std::vector<std::vector<double>> prob_history; // 概率历史
    
    // 构造函数
    DPMM(double alpha, Eigen::MatrixXd X, int maxiter = 100, 
         Inittype type = SingleInit, int n_neighbors = 0) 
        : Alpha(alpha), X(X), Z(X.rows()), Maxiter(maxiter),
          niwparams(NiwParams{(int)X.cols(), 0.0, (double)X.cols(), 
                   Eigen::MatrixXd::Identity(X.cols(), X.cols())}) {
        D = X.cols();
        Probs = std::vector<double>(X.rows(), 0);
        
        // 根据初始化类型选择初始化方法
        if(type == KnnInit) {
            knnInitialization(Z, n_neighbors);  // K近邻初始化
        } else {
            singletonInitialization(Z);          // 单样本初始化
        }
    }

    // 单样本初始化：每个样本作为独立簇
    void singletonInitialization(std::vector<int>& Z) {
        for(size_t i = 0; i < Z.size(); ++i) {
            Z[i] = i; // 每个点自己形成一个簇
            cluster_stats.emplace_back(D, niwparams); // 创建新簇
            cluster_stats[i].addData(X.row(i));       // 添加数据
        }
    }

    // K近邻初始化
    void knnInitialization(std::vector<int>& Z, int K) {
        int n_samples = X.rows();
        Z.resize(n_samples, -1);
        
        // Step 1: 使用KNN进行最近邻搜索
        KNN knn(X);
        std::vector<std::vector<int>> indices;
        std::vector<std::vector<double>> distances;
        knn.knnSearch(K, indices, distances);

        // Step 2: 贪心策略分配簇标签
        int current_label = 0;
        std::vector<bool> visited(n_samples, false);

        for (int i = 0; i < n_samples; ++i) {
            if (!visited[i]) {
                Z[i] = current_label;
                visited[i] = true;
                
                // 将邻居分配到同一簇
                for (int neighbor : indices[i]) {
                    if (!visited[neighbor]) {
                        Z[neighbor] = current_label;
                        visited[neighbor] = true;
                    }
                }
                ++current_label;
            }
        }

        // 初始化簇统计信息
        int max_label = *std::max_element(Z.begin(), Z.end());
        cluster_stats.clear();
        for (int k = 0; k <= max_label; ++k) {
            cluster_stats.emplace_back(D, niwparams);
        }

        // 将数据分配到对应簇
        for (int i = 0; i < X.rows(); ++i) {
            cluster_stats[Z[i]].addData(X.row(i));
        }
    }

    // 添加数据点到指定簇
    void add_xi(int i) {
        int k = Z[i];
        if (k >= 0 && k < (int)cluster_stats.size()) {
            cluster_stats[k].addData(X.row(i));
        }
    }

    // 从簇中移除数据点
    void remove_xi(int i) {
        int k = Z[i];
        if (k >= 0 && k < (int)cluster_stats.size()) {
            cluster_stats[k].removeData(X.row(i));
            
            // 如果簇为空则删除
            if (cluster_stats[k].count <= 0) {
                cluster_stats.erase(cluster_stats.begin() + k);
                
                // 更新簇标签（大于k的标签减1）
                for (auto& z : Z) {
                    if (z > k) z--;
                }
            }
        }
    }

    // 计算数据点分配到现有簇的条件概率
    double condition_existK(int i, Cluster K){
        int N_i = K.count;
        Eigen::VectorXd x_i = X.row(i);
        
        // CRP概率 + 簇似然
        return std::log(N_i / (X.rows() + Alpha - 1)) + K.LogPosteriorPDF(x_i);
    }

    // 计算数据点分配到新簇的条件概率
    double condition_newK(int i, Cluster K){
        Eigen::VectorXd x_i = X.row(i);
        
        // CRP概率 + 先验似然
        return std::log(Alpha / (X.rows() + Alpha - 1)) + K.LogPosteriorPDF(x_i);
    }

    // 对数值进行softmax归一化
    Eigen::VectorXd softmax_normalize(const Eigen::VectorXd& log_values) {
        if (log_values.size() == 0) {
            throw std::runtime_error("Empty log values in softmax");
        }

        // 数值稳定性的softmax实现
        double max_log = log_values.maxCoeff();    
        Eigen::VectorXd exp_values = (log_values.array() - max_log).exp();
        return exp_values / exp_values.sum();
    }

    // 根据概率分布进行采样
    int sample(const std::vector<double>& probs) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::discrete_distribution<> dist(probs.begin(), probs.end());
        return dist(gen);  // 返回采样结果
    }

    // 手动选择簇（调试用）
    int pick(const std::vector<double>& probs){
        for(int i=0; i<probs.size(); i++){
            std::cout<<i<<" "<<probs[i]<<std::endl;
        }
        int id;
        std::cin>>id;
        return id;
    }

    // 选择概率最大的簇
    int argmax(const std::vector<double>& probs) {
        if (probs.empty()) {
            throw std::invalid_argument("Probability vector is empty.");
        }
        return std::distance(probs.begin(), 
                           std::max_element(probs.begin(), probs.end()));
    }

    // 更新簇分配（Gibbs采样一步）
    void update(){
        for(int i=0; i<X.rows(); i++){
            // 1. 从当前分配中移除点i
            remove_xi(i);

            // 2. 计算分配到各现有簇的概率
            std::vector<double> log_weights;
            for (size_t k = 0; k < cluster_stats.size(); ++k) {
                log_weights.push_back(condition_existK(i, cluster_stats[k]));
            }

            // 3. 计算分配到新簇的概率
            Cluster new_K = Cluster(D, niwparams);
            log_weights.push_back(condition_newK(i, new_K));

            // 4. Softmax归一化得到概率分布
            Eigen::VectorXd probs = softmax_normalize(
                Eigen::Map<Eigen::VectorXd>(log_weights.data(), log_weights.size()));
            
            // 5. 根据概率采样新分配
            std::vector<double> probs_std(probs.data(), probs.data() + probs.size());
            int new_cluster_id = sample(probs_std);
            
            // 6. 记录概率和分配结果
            Probs[i] = probs_std[new_cluster_id];
            
            // 7. 处理新簇情况
            if (new_cluster_id == static_cast<int>(cluster_stats.size())) {
                cluster_stats.emplace_back(D, niwparams);
                Z[i] = cluster_stats.size();
            } else {
                Z[i] = new_cluster_id;
            }
            
            // 8. 将点添加到新分配的簇
            add_xi(i);
        }
    }

    // 运行DPMM聚类
    void start(){
        int iter = 0;
        const int max_history = 3; // 收敛判断窗口大小
        std::vector<std::vector<int>> Z_history; // 分配历史记录
        
        while(iter < Maxiter){
            update();  // 执行一次Gibbs采样
            
            // 记录当前状态
            label_history.push_back(Z);
            prob_history.push_back(Probs);
            Z_history.push_back(Z);
            
            // 保持历史记录长度
            if (Z_history.size() > max_history) {
                Z_history.erase(Z_history.begin());
            }

            // 检查是否收敛（最近max_history次分配不变）
            bool converged = true;
            for (int i = 1; i < Z_history.size(); ++i) {
                if (Z_history[i] != Z_history[i - 1]) {
                    converged = false;
                    break;
                }
            }

            if (converged && Z_history.size() >= max_history) {
                std::cout << "Converged after " << iter << " iterations." << std::endl;
                break;
            }
            iter++;
        }
    }
};

#endif // DPMM_H