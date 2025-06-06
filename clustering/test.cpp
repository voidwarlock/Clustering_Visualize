#include "Affinity_Propagation.h"
#include "Agglomerative.h"
#include "DBSCAN.h"
#include "DPMM.h"
#include "K_Means.h"
#include "Spectral.h"


enum ClusterType {k_means, dbscan, agglomerative, dpmm, affinity_propagation, spectral};

// 输入参数说明：
//ClusteringInput：
        // 所有聚类算法通用输入                        double[][] X; 数据矩阵
        // K-Means 专用                              int K; 聚类数量（必须指定）
        // DBSCAN 专用                               double eps;  邻域半径    int minpts;  核心点所需的最小邻域样本数
        // 层次聚类Agglomerative专用                  int nClusters;   最终需要的聚类数量 
        // 狄利克雷过程混合模型DPMM专用                double alpha;   浓度参数（控制自动发现的聚类数量）
        // 亲和力传播Affinity_Propagation专用         double damping;  阻尼系数 (0.5-1)    double preference; 偏好值（控制聚类数量，越高聚类越多） double tol; 误差
        // 谱聚类 (Spectral) 专用                     int K;   聚类数量       norm;  归一化    sigma;  计算W有用

int main(){
    Eigen::MatrixXd X(11, 2);
    X << 5.0,1.0,
        5.0,1.2,
        5.1,1.1,
        4.9,0.9,
        15.0,11.0,
        15.1,11.1,
        15.2,11.3,
        14.9,10.9,
        1.0,5.0,
        1.1,5.1,
        0.9,4.9;

    ClusterType type = k_means;
    if(type == k_means){
        int K = 3;
        int Maxiter = 20;
        K_Means c = K_Means(K, X, Maxiter);
        c.start();
    }
    if(type == dbscan){
        double eps = 1.0;
        int minpts = 1;
        DBSCAN c = DBSCAN(eps, minpts, X);
        c.start();
    }
    if(type == agglomerative){
        int numclusters = 1;
        Agglomerative c = Agglomerative(X, numclusters);
        c.start();
    }
    if(type == dpmm){
        double alpha = 1.0;
        DPMM c = DPMM(alpha, X);
        c.start();
    }
    if(type == affinity_propagation){
        double dampling = 0.5;
        double Tol = 1e-5;
        AffinityPropagation c = AffinityPropagation(dampling, Tol, X, MEDIAN);
        c.start();
    }
    if(type == spectral){
        int K = 3;
        Norm type = NoNorm;
        Spectral c = Spectral(K, X, type);
        c.start();
    }
}