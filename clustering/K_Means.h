#ifndef K_MEANS_H
#define K_MEANS_H

#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <algorithm> // for std::random_shuffle or std::shuffle
#include <random>    // for std::default_random_enginez


class K_Means
{
private:
    int K;
    int Maxiter;
    double tol;
    Eigen::MatrixXd X;
    Eigen::MatrixXd Center;
    
    
public:
    std::vector<int> labels;
    std::vector<std::vector<int>> label_history;
    std::vector<std::vector<double>> centers;
    std::vector<std::vector<std::vector<double>>> center_history;

    K_Means(Eigen::MatrixXd x):X(x){}

    K_Means(int K, Eigen::MatrixXd x, int maxiter = 20, double tor = 1e-6) : K(K), X(x), Maxiter(maxiter), tol(tol){
        Center = Eigen::MatrixXd(K,x.cols());
        labels = std::vector<int>(x.rows());
        Init();
    }

    void Init(){
        std::vector<int> indices(X.rows());
        for (int i = 0; i < X.rows(); ++i) {
            indices[i] = i;
        }
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(indices.begin(), indices.end(), gen);

        for (int i = 0; i < K; ++i) {
            Center.row(i) = X.row(indices[i]);
        }
    }

    Eigen::MatrixXd distance() {
        // 计算X各行范数平方 (N×1)
        Eigen::VectorXd X_norms = X.rowwise().squaredNorm();
        
        // 计算Center各行范数平方 (K×1)
        Eigen::VectorXd C_norms = Center.rowwise().squaredNorm();
        
        // 计算点积矩阵 (N×K)
        Eigen::MatrixXd dot_products = X * Center.transpose();
        
        // 计算距离平方 (广播相加)
        Eigen::MatrixXd dists_sq = (-2 * dot_products).rowwise() + C_norms.transpose();
        dists_sq = dists_sq.colwise() + X_norms;

        return dists_sq.cwiseMax(0).array().sqrt();
    }

    bool update(Eigen::MatrixXd dists){
        Eigen::MatrixXd NewCenter = Eigen::MatrixXd(K,X.cols());
        std::vector<int> counts(K, 0); 

        NewCenter.setZero(); 

        for (int i = 0; i < X.rows(); ++i){
            int index;
            dists.row(i).minCoeff(&index);
            //std::cout<<index<<std::endl;
            labels[i] = index;
            NewCenter.row(index) += X.row(i);
            counts[index]++;
        }

        for (int k = 0; k < K; ++k) {
            if (counts[k] > 0) { 
                NewCenter.row(k) /= static_cast<double>(counts[k]);
            }
        }
        if(((NewCenter - Center).cwiseAbs().array() < tol).all()){
            return true;
        }
        else{

            //std::cout<<NewCenter<<std::endl;
            Center = NewCenter;
            //std::cout<<std::endl;
            //std::cout<<Center<<std::endl;
            return false;
        }
    }
    double computeCost() {
        double cost = 0.0;
        for (int i = 0; i < X.rows(); ++i) {
            int label = labels[i];
            double dist = (X.row(i) - Center.row(label)).norm();
            cost += dist * dist;  
        }
        return cost;
    }

    void get_center(){
        centers.clear();
        for (int k = 0; k < Center.rows(); ++k) {
            std::vector<double> center_point;
            for (int j = 0; j < Center.cols(); ++j) {
                center_point.push_back(Center(k, j));
            }
            centers.push_back(center_point);
        }
    }

    void start(){
        int i=0;
        label_history.push_back(labels);
        get_center();
        center_history.push_back(centers);
        while(i<Maxiter){

            if(update(distance())){
                break;
            }
            label_history.push_back(labels);

            get_center();
            center_history.push_back(centers);
            //double current_cost = computeCost();
            //std::cout << "Iteration " << i+1 << " - Cost: " << current_cost << std::endl;
            //for(int i=0; i<labels.size(); i++){
            //    std::cout<<X.row(i)<<"  :"<<labels[i]<<std::endl;
            //}
            i++;
        }
    }

};

#endif