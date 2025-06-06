#ifndef SPECTRAL_H
#define SPECTRAL_H

#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <Eigen/Eigenvalues>
#include <algorithm> // for std::random_shuffle or std::shuffle
#include <random>    // for std::default_random_engine
#include "K_Means.h"

//Input: K   X   norm  sigma

enum Norm {NoNorm, RW, SYM};


class Spectral{
private:
    int K;
    double Sigma;
    Eigen::MatrixXd X;
    Norm Normstyle;

public:
    std::vector<int> labels;
    std::vector<std::vector<int>> label_history;
    Spectral(int k, Eigen::MatrixXd x, Norm normstyle = NoNorm, double sigma = 1.0) : K(k), X(x), Normstyle(normstyle), Sigma(sigma){}

    Eigen::MatrixXd distance() {
        Eigen::VectorXd row_norms = X.rowwise().squaredNorm();
        Eigen::MatrixXd dot_products = X * X.transpose();
        Eigen::MatrixXd dists = (-2 * dot_products).rowwise() + row_norms.transpose();
        dists = dists.colwise() + row_norms;
        return dists.array().sqrt();
    }

    Eigen::MatrixXd getW(double sigma = 1.0){
        Eigen::MatrixXd dists = distance();

        Eigen::MatrixXd W = (-dists / (2 * sigma * sigma)).array().exp();
        W.diagonal().setZero();
        //std::cout<<W<<std::endl;
        return W;
    }

    Eigen::MatrixXd getD(Eigen::MatrixXd W){
        Eigen::MatrixXd D = W.rowwise().sum().asDiagonal();
        return D;
    }

    Eigen::MatrixXd getL(Eigen::MatrixXd W, Eigen::MatrixXd D){
        Eigen::MatrixXd L = D - W;
        return L;
    }

    Eigen::MatrixXd getL_sym(Eigen::MatrixXd W, Eigen::MatrixXd D) {
        // 计算 D^(-1/2)
        Eigen::MatrixXd D_inv_sqrt = D;
        for (int i = 0; i < D.rows(); ++i) {
            double diag_val = D(i, i);
            if (diag_val > 0) { // 确保不除以零
                D_inv_sqrt(i, i) = 1.0 / std::sqrt(diag_val);
            } else {
                D_inv_sqrt(i, i) = 0;
            }
        }

        // 对称归一化拉普拉斯矩阵 L_sym = I - D^(-1/2) * W * D^(-1/2)
        return Eigen::MatrixXd::Identity(D.rows(), D.cols()) - D_inv_sqrt * W * D_inv_sqrt;
    }

    Eigen::MatrixXd getL_rw(Eigen::MatrixXd W, Eigen::MatrixXd D) {
        // 计算 D^(-1)
        Eigen::MatrixXd D_inv = D;
        for (int i = 0; i < D.rows(); ++i) {
            double diag_val = D(i, i);
            if (diag_val > 0) { // 确保不除以零
                D_inv(i, i) = 1.0 / diag_val;
            } else {
                D_inv(i, i) = 0;
            }
        }

        // 随机游走归一化拉普拉斯矩阵 L_rw = I - D^(-1) * W
        return Eigen::MatrixXd::Identity(D.rows(), D.cols()) - D_inv * W;
    }

    Eigen::MatrixXd getEigen(Eigen::MatrixXd L) {
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> es(L);
        
        // 安全处理 K 的取值
        int max_possible_k = L.cols() - 1;  // 跳过第一个特征向量（对应特征值0）
        int effective_k = std::min(K, max_possible_k);
        effective_k = std::max(1, effective_k);  // 至少取1个特征向量

        // 检查是否有足够的特征向量
        if (es.eigenvectors().cols() < effective_k + 1) {
            throw std::runtime_error("Not enough eigenvectors for K=" + std::to_string(K));
        }

        // 提取前K个最小非零特征值对应的特征向量（跳过第一个）
        Eigen::MatrixXd U = es.eigenvectors().block(0, 1, L.rows(), effective_k);
        return U;
    }

    void start(){
        Eigen::MatrixXd W = getW(Sigma);
        Eigen::MatrixXd D = getD(W);
        Eigen::MatrixXd L;
        if(Normstyle == RW){
            L = getL_rw(W, D);
        }
        else if(Normstyle == SYM){
            L = getL_sym(W, D);
        }
        else{
            L = getL(W, D);
        }
        
        //std::cout<<W<<std::endl;
        //std::cout<<D<<std::endl;
        //std::cout<<L<<std::endl;
        Eigen::MatrixXd U = getEigen(L);
//
        K_Means k_means = K_Means(K, U);
        k_means.start();
//
        labels = k_means.labels;
        label_history = k_means.label_history;
        //for(int i=0; i<labels.size(); i++){
        //    std::cout<<X.row(i)<<"  :"<<labels[i]<<std::endl;
        //}

    }


};

#endif
