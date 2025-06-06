#ifndef KNN_H
#define KNN_H


#include <Eigen/Dense>
#include <vector>
#include <queue>
using MatrixXd = Eigen::MatrixXd;
using VectorXd = Eigen::VectorXd;

class KNN {
public:


    struct Neighbor {
        int index;
        double distance;

        bool operator<(const Neighbor& other) const {
            return distance < other.distance; // 用于最大堆排序
        }
    };

    // 构造函数：传入训练数据
    explicit KNN(const MatrixXd& data): data_(data){
        if (data.rows() == 0 || data.cols() == 0) {
            throw std::invalid_argument("Data matrix is empty.");
        }
    }

    // 对单个点进行 KNN 查询
    void knnQuery(const VectorXd& query, int K,
                  std::vector<int>& indices,
                  std::vector<double>& distances) const{
        int n_samples = data_.rows();
        int dim = data_.cols();

        if (query.size() != dim) {
            throw std::invalid_argument("Query dimension mismatch.");
        }

        std::priority_queue<Neighbor> heap;

        for (int j = 0; j < n_samples; ++j) {
            VectorXd xj = data_.row(j).transpose();  // 变成列向量
            double dist = (query - xj).squaredNorm();

            if (heap.size() < K) {
                heap.push({j, dist});
            } else if (dist < heap.top().distance) {
                heap.pop();
                heap.push({j, dist});
            }
        }

        // 提取并排序
        std::vector<std::pair<double, int>> temp;
        while (!heap.empty()) {
            const Neighbor& n = heap.top();
            temp.emplace_back(n.distance, n.index);
            heap.pop();
        }

        std::sort(temp.begin(), temp.end());

        // 输出结果
        indices.resize(K);
        distances.resize(K);
        for (int k = 0; k < K; ++k) {
            distances[k] = std::sqrt(temp[k].first);
            indices[k] = temp[k].second;
        }
    }

    // 对整个数据集进行 KNN 查询
    void knnSearch(int K,
                   std::vector<std::vector<int>>& all_indices,
                   std::vector<std::vector<double>>& all_distances){
                    int n_samples = data_.rows();
        all_indices.resize(n_samples);
        all_distances.resize(n_samples);

        for (int i = 0; i < n_samples; ++i) {
            knnQuery(data_.row(i), K, all_indices[i], all_distances[i]);
        }
    }

private:
    MatrixXd data_;
};

#endif