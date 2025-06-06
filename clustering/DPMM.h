#ifndef DPMM_H
#define DPMM_H

#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <algorithm> // for std::random_shuffle or std::shuffle
#include <random>    // for std::default_random_engine
#include "KNN.h"

// Input:  X, Alpha
enum Inittype {SingleInit, KnnInit};

struct NiwParams{
    Eigen::VectorXd mu0;   // 先验均值向量
    double kappa0;         // 先验精度
    int nu0;            // 自由度
    Eigen::MatrixXd Psi0;  // 缩放矩阵

    NiwParams(int dim, double kappa, double nu, const Eigen::MatrixXd& psi)
        : mu0(Eigen::VectorXd::Zero(dim)), kappa0(kappa), nu0(nu), Psi0(psi) {}
};


class Cluster{
private:
    std::vector<Eigen::VectorXd> datas;
    int D;
    NiwParams niwparams;

    Eigen::VectorXd mean;
    Eigen::MatrixXd covariance;

    Eigen::MatrixXd meanError;
    int meanDf;

    Eigen::VectorXd sum;  // sum_x: 样本和
    Eigen::MatrixXd sq_sum; // sum_xxT: 平方和矩阵

    bool cache_flag;
    double cache_covariance_determinant;
    Eigen::MatrixXd cache_covariance_inverse;

public:
    int count;
    Cluster() : D(0), niwparams(NiwParams(0, 0.0, 0, Eigen::MatrixXd::Identity(0, 0))) {
        mean = Eigen::VectorXd::Zero(0);
        covariance = Eigen::MatrixXd::Identity(0, 0);

        sum = Eigen::VectorXd::Zero(0);
        sq_sum = Eigen::MatrixXd::Zero(0, 0);

        meanError = Eigen::MatrixXd::Zero(0, 0);
        meanDf = 0;

        cache_flag = false;
        count = 0;
    }

    Cluster(int D, NiwParams niwParams) : D(D), niwparams(niwParams){
        mean = Eigen::VectorXd(D);
        covariance = Eigen::MatrixXd::Identity(D, D);

        sum = Eigen::VectorXd::Zero(D);
        sq_sum = Eigen::MatrixXd::Zero(D, D);

        meanError = calculateMeanError(niwParams.Psi0, niwParams.kappa0, niwParams.nu0);
        meanDf = std::max(0, niwParams.nu0 - D + 1);
    }

    Eigen::MatrixXd calculateMeanError(Eigen::MatrixXd Psi, int kappa, int nu){
        return Psi * (1.0 / (kappa * nu - D + 1.0));
    }

    void addData(Eigen::VectorXd data){
        count++;
        //std::cout<<"sum: "<< sum<<std::endl;
        sum += data;
        sq_sum += data * data.transpose();
        datas.push_back(data); 
        updateParameters();
    }

    void removeData(Eigen::VectorXd data){
        auto it = std::find(datas.begin(), datas.end(), data);
        int index;
        if(it==datas.end()){
            return;
        }else{
            index = std::distance(datas.begin(), it);
        }
        count--;
        sum -= data;
        sq_sum -= data * data.transpose();
        datas.erase(datas.begin() + index);

        updateParameters();
    }

    void updateParameters(){
        int n = datas.size();
        if(n<=0){
            return;
        }
        int kappa = niwparams.kappa0 + n;
        int nu = niwparams.nu0 + n;

        Eigen::VectorXd mu = sum / n;
        Eigen::VectorXd mu_mu = mu - niwparams.mu0;

        Eigen::MatrixXd C = sq_sum - ((mu * mu.transpose()) * n);

        Eigen::MatrixXd psi = niwparams.Psi0 + C + ((mu_mu * mu_mu.transpose()) * (niwparams.kappa0 * n / (double)kappa));

        mean = (niwparams.mu0 * niwparams.kappa0 + mu * n) / (double)kappa;
        //std::cout<<"mean: "<<mean<<std::endl;
        covariance = psi * ((kappa + 1.0) / (kappa * (nu - D + 1.0)));

        cache_flag = false;

        meanError = calculateMeanError(psi, kappa, nu);

        meanDf = std::max(0, nu - D + 1);

    }

    double LogPosteriorPDF(Eigen::VectorXd data){
        Eigen::VectorXd x_mu = data - mean;
        if(!cache_flag){
            Eigen::FullPivLU<Eigen::MatrixXd> lu(covariance);
            cache_covariance_determinant = lu.determinant();
            cache_covariance_inverse = lu.inverse();
            cache_flag = true;
        }
        
        double determinant=cache_covariance_determinant;
        Eigen::MatrixXd invCovariance = cache_covariance_inverse;

        double x_muInvSx_muT = x_mu.transpose() * invCovariance * x_mu;

        //double normConst = 1.0 / (std::pow(2 * M_PI, D / 2.0) * std::pow(determinant, 0.5));
        
        //return -0.5 * x_muInvSx_muT + std::log(normConst);

        double log_prob = std::lgamma((meanDf + D)/2.0) 
                    - std::lgamma(meanDf/2.0) 
                    - (D/2.0) * std::log(meanDf * M_PI) 
                    - 0.5 * std::log(cache_covariance_determinant)
                    - ((meanDf + D)/2.0) * std::log(1.0 + x_muInvSx_muT / meanDf);
        return log_prob;
    }

};

class DPMM
{
private:
    int D;
    int Maxiter;
    double Alpha;
    Eigen::MatrixXd X;
    NiwParams niwparams;
    std::vector<Cluster> cluster_stats;

public:
    std::vector<int> Z;
    std::vector<double> Probs;
    std::vector<std::vector<int>> label_history;
    std::vector<std::vector<double>> prob_history;
    
    DPMM(double alpha, Eigen::MatrixXd X, int maxiter = 100, Inittype type = SingleInit, int n_neighbors = 0) : Alpha(alpha), X(X), Z(X.rows()), Maxiter(maxiter),
    niwparams(NiwParams{(int)X.cols(), 0.0, (double)X.cols(), Eigen::MatrixXd::Identity(X.cols(), X.cols())}){
        D = X.cols();
        Probs = std::vector<double>(X.rows(), 0);
        if(type == KnnInit){
            knnInitialization(Z, n_neighbors);
        }else{
            singletonInitialization(Z);
        }
    }

    void singletonInitialization(std::vector<int>& Z) {
        for(size_t i = 0; i < Z.size(); ++i) {
            Z[i] = i; // 每个点自己形成一个簇
            cluster_stats.emplace_back(D, niwparams);
            cluster_stats[i].addData(X.row(i));
        }
    }

    void knnInitialization(std::vector<int>& Z, int K) {
        int n_samples = X.rows();
        Z.resize(n_samples, -1);

        // Step 1: 使用 KNNIndex 进行最近邻搜索
        KNN knn(X);

        std::vector<std::vector<int>> indices;
        std::vector<std::vector<double>> distances;

        knn.knnSearch(K, indices, distances); // 假设你已实现 knnSearch 方法

        //// Step 2: 贪心策略分配簇标签
        int current_label = 0;
        std::vector<bool> visited(n_samples, false);

        for (int i = 0; i < n_samples; ++i) {
            if (!visited[i]) {
                Z[i] = current_label;
                visited[i] = true;

                // 把邻居也合并进来
                for (int neighbor : indices[i]) {
                    if (!visited[neighbor]) {
                        Z[neighbor] = current_label;
                        visited[neighbor] = true;
                    }
                }

                ++current_label;
            }
        }

        std::cout << "Initialized with " << current_label << " clusters." << std::endl;

        int max_label = *std::max_element(Z.begin(), Z.end());

        cluster_stats.clear();
        for (int k = 0; k <= max_label; ++k) {
            cluster_stats.emplace_back(D, niwparams);
        }

        for (int i = 0; i < X.rows(); ++i) {
            std::cout<<Z[i]<<" ";
            Eigen::VectorXd xi = X.row(i);
            cluster_stats[Z[i]].addData(xi);
        }
        std::cout<<std::endl;
    }

    void add_xi(int i) {
        int k = Z[i];
        if (k >= 0 && k < (int)cluster_stats.size()) {
            cluster_stats[k].addData(X.row(i));
        }
    }

    void remove_xi(int i) {
        int k = Z[i];
        if (k >= 0 && k < (int)cluster_stats.size()) {
            cluster_stats[k].removeData(X.row(i));

            if (cluster_stats[k].count <= 0) {
                // 删除第 k 个簇
                cluster_stats.erase(cluster_stats.begin() + k);

                // 更新 Z 中大于等于 k 的标签
                for (auto& z : Z) {
                    if (z > k) z--;
                }
            }
        }
    }

    double condition_existK(int i, Cluster K){
        int N_i = K.count;
        Eigen::VectorXd x_i = X.row(i);

        double log_marginal_likelihood = K.LogPosteriorPDF(x_i);
        //std::cout<<"log_marginal_likelihood: "<<log_marginal_likelihood<<std::endl;
        return std::log(N_i / (X.rows() + Alpha - 1)) + log_marginal_likelihood;
    }


    double condition_newK(int i, Cluster K){
        Eigen::VectorXd x_i = X.row(i);

        double log_marginal_likelihood = K.LogPosteriorPDF(x_i);
                
        return std::log(Alpha / (X.rows() + Alpha - 1)) + log_marginal_likelihood;
    }


    Eigen::VectorXd softmax_normalize(const Eigen::VectorXd& log_values) {
        if (log_values.size() == 0) {
            throw std::runtime_error("Empty log values in softmax");
        }

        double max_log = log_values.maxCoeff();    
        Eigen::VectorXd exp_values = (log_values.array() - max_log).exp();

        return exp_values / exp_values.sum();
    }

    int sample(const std::vector<double>& probs) {
        // 创建一个随机设备并用它初始化随机数引擎
        std::random_device rd;
        std::mt19937 gen(rd());

        std::discrete_distribution<> dist(probs.begin(), probs.end());
        // 采样一个类别索引
        return dist(gen);
    }

    int pick(const std::vector<double>& probs){
        for(int i=0; i<probs.size(); i++){
            std::cout<<i<<" ";
            std::cout<<probs[i]<<std::endl;
        }
        int id;
        std::cin>>id;
        return id;
    }

    int argmax(const std::vector<double>& probs) {
        if (probs.empty()) {
            throw std::invalid_argument("Probability vector is empty.");
        }

        int max_index = 0;
        double max_value = probs[0];

        for (size_t i = 1; i < probs.size(); ++i) {
            if (probs[i] > max_value) {
                max_value = probs[i];
                max_index = i;
            }
        }

        return max_index;
    }

    void update(){
        
        for(int i=0; i<X.rows(); i++){
            remove_xi(i);

            std::vector<double> log_weights;
            int n=0;
            for (size_t k = 0; k < cluster_stats.size(); ++k) {
                //std::cout<<"cluster: "<<n<<std::endl;
                log_weights.push_back(condition_existK(i, cluster_stats[k]));
                n++;
            }

            Cluster new_K = Cluster(D, niwparams);
            log_weights.push_back(condition_newK(i, new_K));


            Eigen::Map<Eigen::VectorXd> eigen_log_weights(log_weights.data(), log_weights.size());
            

            Eigen::VectorXd probs = softmax_normalize(eigen_log_weights);

            std::vector<double> probs_std(probs.data(), probs.data() + probs.size());

            int new_cluster_id = sample(probs_std);

            Probs[i] = probs_std[new_cluster_id];
            //int new_cluster_id = pick(probs_std);
            //int new_cluster_id = argmax(probs_std);
            //std::cout<<"choose_prob: "<<probs_std[new_cluster_id]<<std::endl;
            
            if (new_cluster_id == static_cast<int>(cluster_stats.size())) {
                cluster_stats.emplace_back(D, niwparams);
                Z[i] = cluster_stats.size();
            }else {
                Z[i] = new_cluster_id;
            }
            add_xi(i);
        }
    }
    void start(){
        int iter = 0;
        const int max_history = 3; // 连续 3 次无变化视为收敛
        std::vector<std::vector<int>> Z_history;
        while(iter < Maxiter){
            update();

            std::vector<int> current_Z = Z;
            label_history.push_back(current_Z);
            prob_history.push_back(Probs);
        // 添加到历史记录
            Z_history.push_back(current_Z);
            if (Z_history.size() > max_history) {
                Z_history.erase(Z_history.begin()); // 只保留最近几次
            }

        // 判断是否收敛
            bool converged = true;
            for (int i = 1; i < Z_history.size(); ++i) {
                if (Z_history[i] != Z_history[i - 1]) {
                    converged = false;
                    break;
                }
            }

        // 输出当前 Z
            //std::cout << "Iteration " << iter << ": ";
            //for (int z : Z) std::cout << z << " ";
            //std::cout << std::endl;

            if (converged && Z_history.size() >= max_history) {
                std::cout << "Converged after " << iter << " iterations." << std::endl;
                break;
            }
            iter++;
        }

    }

};


#endif