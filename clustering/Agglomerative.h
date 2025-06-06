#ifndef AGGLOMERATIVE_H
#define AGGLOMERATIVE_H


#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <algorithm> // for std::random_shuffle or std::shuffle
#include <random>    // for std::default_random_engine
#include <queue>

struct ClusterNode {
    int id;
    int height;
    std::vector<int> ids;       
    ClusterNode* left;
    ClusterNode* right;

    ClusterNode(int _id, int _height = 0, ClusterNode* _left = nullptr, ClusterNode* _right = nullptr)
        : id(_id), height(_height), left(_left), right(_right) {
            ids.push_back(id);
        }
};

struct ClusterPair {
    ClusterNode* node1;
    ClusterNode* node2;
    double distance;

    bool operator<(const ClusterPair& other) const {
        return distance > other.distance; 
    }
};

class Agglomerative{
private:
    std::vector<ClusterNode*> nodes;
    Eigen::MatrixXd X;
    Eigen::MatrixXd dists;
    std::priority_queue<ClusterPair> PossibleClusters;
    std::vector<bool> is_valid;
    int Numclusters;

public:
    std::vector<int> labels;
    std::vector<ClusterNode*> roots;
    std::vector<std::vector<int>> label_history;
    std::vector<std::vector<ClusterNode*>> root_history;
    std::vector<int> num_history;
    Agglomerative(Eigen::MatrixXd x, int numclusters) :X(x), Numclusters(numclusters){
        is_valid = std::vector<bool>(x.rows(), true);
        for(int i=0; i<x.rows(); i++){
            ClusterNode* node = new ClusterNode(i);
            nodes.push_back(node);
            roots.push_back(node);
        }
    }

    void distance() {
        Eigen::VectorXd row_norms = X.rowwise().squaredNorm();
        Eigen::MatrixXd dot_products = X * X.transpose();
        dists = (-2 * dot_products).rowwise() + row_norms.transpose();
        dists = dists.colwise() + row_norms;
        dists = dists.array().sqrt();
    }

    void init_PossibleCLusters(){
        for(int i=0; i<dists.rows(); i++){
            for(int j=i+1; j<dists.cols(); j++){
                PossibleClusters.push({nodes[i], nodes[j], dists(i, j)});
            }
        }
    }

    double update_average_linkage_distance(ClusterNode* node1, ClusterNode* node2){
        int count = 0;
        double total = 0.0;
        #pragma omp parallel for reduction(+:sum,count)
        for (int i : node1->ids) {
            for (int j : node2->ids) {
                total += dists(i, j);
                ++count;
            }
        }
        return count > 0 ? total / count : std::numeric_limits<double>::infinity();
    }

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

    void update(){
        int currentid = X.rows();
        int N = X.rows();
        while(!PossibleClusters.empty()){
            ClusterPair curr = PossibleClusters.top();
            PossibleClusters.pop();

            if (!is_valid[curr.node1->id] || !is_valid[curr.node2->id]) continue;

            roots.erase(std::remove(roots.begin(), roots.end(), curr.node1), roots.end());
            roots.erase(std::remove(roots.begin(), roots.end(), curr.node2), roots.end());

            is_valid[curr.node1->id] = false;
            is_valid[curr.node2->id] = false;

            ClusterNode* new_node = new ClusterNode(currentid, 0, curr.node1, curr.node2);
            new_node->height = std::max(curr.node1->height,curr.node2->height) + 1;
            new_node->ids = curr.node1->ids;
            new_node->ids.insert(new_node->ids.end(), curr.node2->ids.begin(), curr.node2->ids.end());

            //std::cout<<"{";
            //for(int i=0; i<new_node->ids.size(); i++){
//
            //    std::cout<<"{";
            //    std::cout<<X.row(new_node->ids[i])<<" ";
            //    std::cout<<"}";
            //    
            //}
            //std::cout<<"}"<<std::endl;

            for (size_t i = 0; i < nodes.size(); ++i) {
                if (is_valid[i] && nodes[i] != curr.node1 && nodes[i] != curr.node2) {
                    double avg_dist = update_average_linkage_distance(new_node, nodes[i]);
                    PossibleClusters.push({new_node, nodes[i], avg_dist});
                }
            }

            roots.push_back(new_node);
            nodes.push_back(new_node);
            is_valid.push_back(true);

            std::vector<int> temp_label = Assign_Labels();
            label_history.push_back(temp_label);    
            root_history.push_back(roots);

            N -= 1;
            num_history.push_back(N);
            if(N==Numclusters){
                labels = temp_label;
            }
            currentid++;
        }
    }

    void start() {
        distance();
        init_PossibleCLusters();
        update();
    }

};

#endif