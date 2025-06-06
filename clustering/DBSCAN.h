#ifndef DBSCAN_H
#define DBSCAN_H

#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <algorithm> // for std::random_shuffle or std::shuffle
#include <random>    // for std::default_random_engine

//Input: eps, minpts, X

enum Pointtype {Corepoint, Marginpoint, Noisepoint};

class DBSCAN
{
private:
    double Eps;
    int Minpts;
    Eigen::MatrixXd X;
    
    std::vector<int> density;
    std::vector<std::vector<int>> neighbors;
    std::vector<bool> visited;
public:
    std::vector<Pointtype> point_features;
    std::vector<int> labels;
    std::vector<std::vector<int>> label_history;
    std::vector<std::vector<Pointtype>> point_feature_history;
    DBSCAN(double eps, int minpts, Eigen::MatrixXd X) : Eps(eps), Minpts(minpts), X(X){
        labels = std::vector<int>(X.rows(), -1);
        point_features = std::vector<Pointtype>(X.rows());
        density = std::vector<int>(X.rows());
        neighbors = std::vector<std::vector<int>>(X.rows());
        visited = std::vector<bool>(X.rows(), false);
    }

    void distance() {
        int n = X.rows();
        Eigen::VectorXd row_norms = X.rowwise().squaredNorm();

        // 计算所有点之间的平方距离：dists(i,j) = ||x_i - x_j||^2
        Eigen::MatrixXd dot_products = X * X.transpose();
        Eigen::MatrixXd dists_sq = (-2 * dot_products).rowwise() + row_norms.transpose();
        dists_sq = dists_sq.colwise() + row_norms;

        double eps_sq = Eps * Eps;

        // 构建邻居列表和密度
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != j && dists_sq(i, j) <= eps_sq) {
                    ++density[i];
                    neighbors[i].push_back(j);
                }
            }

            // 设置 point_features
            if (density[i] >= Minpts) {
                point_features[i] = Corepoint;
            } else if (density[i] > 0) {
                point_features[i] = Marginpoint;
            } else {
                point_features[i] = Noisepoint;
            }
        }
    }

    void expandCluster(int point, int label) {
        std::queue<int> queue;
        queue.push(point);
        visited[point] = true;
        labels[point] = label;
        label_history.push_back(labels);
        point_feature_history.push_back(point_features);

        while (!queue.empty()) {
            int current = queue.front();
            queue.pop();
            if (point_features[current] == Corepoint) {
                for (int neighbor : neighbors[current]) {
                    if (!visited[neighbor]) {
                        visited[neighbor] = true;
                        labels[neighbor] = label;
                        label_history.push_back(labels);
                        point_feature_history.push_back(point_features);
                        queue.push(neighbor);
                    }
                }
            }
        }
    }

    void update(){

        int label = 0;
        for(int i=0; i<X.rows(); ++i){
            if(point_features[i]==Corepoint && !visited[i]){
                labels[i] = label;
                visited[i] = true;
                expandCluster(i, label);
                label++;
            }
        }
        
    }

    void start(){
        distance();
        update();
        //for(int i=0; i<labels.size(); i++){
        //    std::cout<<X.row(i)<<"  :"<<labels[i]<<std::endl;
        //}
    }

};


#endif