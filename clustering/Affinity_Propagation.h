#ifndef AFFINITY_PROPAGATION_H
#define AFFINITY_PROPAGATION_H

#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <algorithm> // for std::random_shuffle or std::shuffle
#include <random>    // for std::default_random_engine


#define MEDIAN -114514


class AffinityPropagation{
private:
    double Damping;
    double Tol;
    double Preference;
    int Maxiter;
    Eigen::MatrixXd X;
    Eigen::MatrixXd Responsibility;
    Eigen::MatrixXd Availability;
    Eigen::MatrixXd new_Responsibility;
    Eigen::MatrixXd new_Availability;

public:
    std::vector<int> indexcenters;
    std::vector<std::vector<std::vector<double>>> center_history;
    std::vector<std::vector<double>> centers;
    std::vector<int> labels;
    std::vector<std::vector<int>> label_history;
    AffinityPropagation(double damping, double tol, Eigen::MatrixXd x, double preference = MEDIAN, int maxiter = 1000) : Damping(damping), Tol(tol), X(x), Preference(preference), Maxiter(maxiter){
        Responsibility = Eigen::MatrixXd::Zero(X.rows(), X.rows());
        Availability = Eigen::MatrixXd::Zero(X.rows(), X.rows());
        new_Responsibility = Eigen::MatrixXd::Zero(X.rows(), X.rows());
        new_Availability = Eigen::MatrixXd::Zero(X.rows(), X.rows());
    }

    Eigen::MatrixXd distance() {
        Eigen::VectorXd row_norms = X.rowwise().squaredNorm();
        Eigen::MatrixXd dot_products = X * X.transpose();
        Eigen::MatrixXd dists = (-2 * dot_products).rowwise() + row_norms.transpose();
        dists = dists.colwise() + row_norms;
        return -dists;
    }

    void setPreferenceToMedian(Eigen::MatrixXd& similarity) {
        int n = similarity.rows();
        Eigen::VectorXd off_diag(n * (n - 1));

        // 快速提取所有非对角线元素
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

    void setPreference(Eigen::MatrixXd& similarity) {
        similarity.diagonal().setConstant(Preference);
    }

    void new_res(Eigen::MatrixXd& similarity) {
        Eigen::MatrixXd S_plus_A = similarity + Availability;
        #pragma omp parallel for
        for (int i = 0; i < S_plus_A.rows(); ++i) {
            Eigen::VectorXd row = S_plus_A.row(i);
            
            for (int k = 0; k < S_plus_A.cols(); ++k) {
                double temp = row(k);
                row(k) = -1e9;
                double max_val = row.maxCoeff();
                new_Responsibility(i, k) = similarity(i, k) - max_val;
                row(k) = temp;  
            }
        }
    }

    void new_avai() {
        int n = Responsibility.rows();
        new_Availability = Eigen::MatrixXd::Zero(n, n);

        Eigen::MatrixXd posResponsibility = Responsibility.array().max(0.0);
        Eigen::VectorXd diag_sums = posResponsibility.colwise().sum();

        // 修正：计算sum_{i'≠k} max(0, R(i',k))
        for (int k = 0; k < n; ++k) {
            diag_sums(k) -= std::max(0.0, Responsibility(k, k));
        }

        new_Availability.diagonal() = diag_sums;
        #pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            for (int k = 0; k < n; ++k) {
                if (i == k) continue;
                // 修正：sum_pos 应为 diag_sums(k) - max(0, R(i,k))
                double sum_pos = diag_sums(k) - posResponsibility(i, k);
                new_Availability(i, k) = std::min(0.0, Responsibility(k, k) + sum_pos);
            }
        }

    }
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
            if (r_diff < Tol && a_diff < Tol) {
                std::cout << "Converged at iteration " << i << std::endl;
                break;
            }
            i++;
        }
    }

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

    std::vector<int> Assign_Labels() {
        if (indexcenters.empty()) {
            //std::cerr << "No indexcenters found. Run pick_center() first." << std::endl;
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
        //std::cout<<"indexcenters"<<std::endl;
        //for(int i=0; i<indexcenters.size(); i++){
        //    std::cout<<indexcenters[i]<<"   ";
        //    std::cout<<X.row(indexcenters[i])<<std::endl;
        //}
        //for(int i=0; i<labels.size(); i++){
        //    std::cout<<X.row(indexcenters[i])<<"   ";
        //    std::cout<<labels[i]<<std::endl;
        //    
        //}
    }

};

#endif