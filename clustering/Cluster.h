#ifndef CLUSTER_H
#define CLUSTER_H

#include "Affinity_Propagation.h"
#include "Agglomerative.h"
#include "DBSCAN.h"
#include "DPMM.h"
#include "K_Means.h"
#include "Spectral.h"


enum ClusterType {None, k_means, dbscan, agglomerative, dpmm, affinity_propagation, spectral};

struct ClusteringParams {
    ClusterType clustertype;
    int k;
    double eps;
    int minpts;
    int nClusters;
    double alpha;
    double damping;
    double preference;
    double tol;
    int maxiter;
    double sigma;
    Norm normType;
    Inittype initType;
    int n_neighbors;
};

class ALLCluster{

public:
    Eigen::MatrixXd X;
    ClusteringParams params;
    std::vector<int> labels;
    std::vector<std::vector<double>> centers;
    std::vector<Pointtype> point_features;
    std::vector<double> probs;
    std::vector<ClusterNode*> roots;

    std::vector<std::vector<int>> label_history;
    std::vector<std::vector<std::vector<double>>> center_history;
    std::vector<std::vector<Pointtype>> point_feature_history;
    std::vector<std::vector<double>> prob_history;
    std::vector<std::vector<ClusterNode*>> root_history;
    std::vector<int> num_history;

    ALLCluster(Eigen::MatrixXd x, ClusteringParams Params):X(x), params(Params){}

    void setDatas(Eigen::MatrixXd x){
        X = x;
    }

    void setParams(ClusteringParams Params){
        params = Params;
    }

    void start(){
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
        if(params.clustertype == k_means){
            K_Means c = K_Means(params.k, X, params.maxiter, params.tol);
            c.start();
            labels = c.labels;
            centers = c.centers;

            label_history = c.label_history;
            center_history = c.center_history;
        }
        if(params.clustertype == dbscan){
            DBSCAN c = DBSCAN(params.eps, params.minpts, X);
            c.start();
            labels = c.labels;
            point_features = c.point_features;

            label_history = c.label_history;
            point_feature_history = c.point_feature_history;
        }
        if(params.clustertype == agglomerative){
            Agglomerative c = Agglomerative(X, params.nClusters);
            c.start();
            labels = c.labels;
            roots = c.roots;

            label_history = c.label_history;
            root_history = c.root_history;
            num_history = c.num_history;;
        }
        if(params.clustertype == dpmm){
            DPMM c = DPMM(params.alpha, X, params.maxiter);
            c.start();
            labels = c.Z;
            probs = c.Probs;

            label_history = c.label_history;
            prob_history = c.prob_history;
        }
        if(params.clustertype == affinity_propagation){
            AffinityPropagation c = AffinityPropagation(params.damping, params.tol, X, params.preference, params.maxiter);
            c.start();
            labels = c.labels;
            centers = c.centers;

            label_history = c.label_history;
            center_history = c.center_history;
        }
        if(params.clustertype == spectral){
            Spectral c = Spectral(params.k, X, params.normType);
            c.start();
            labels = c.labels;

            label_history = c.label_history;
        }
        //if(params.clustertype != None){
        //    for(int i=0; i<labels.size(); i++){
        //        std::cout<<X.row(i)<<": "<<labels[i]<<std::endl;
        //    }
        //}
    }

};


#endif