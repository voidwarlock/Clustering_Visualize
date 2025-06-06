// dataloader.cpp
#include "LoadData.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>


DataLoader::DataLoader(const std::string &filename)
    : filename_(filename) {}

std::vector<std::pair<double, double>> DataLoader::load() {
    std::ifstream file(filename_);
    std::vector<std::pair<double, double>> dataPoints;

    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename_ << std::endl;
        return dataPoints; // 返回空 vector
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        double x, y;

        // 尝试读取两个 double
        if (ss >> x >> y) {
            dataPoints.emplace_back(x, y);
        } else {
            std::cerr << "数据解析失败: " << line << std::endl;
        }
    }

    file.close();
    return dataPoints;
}

QList<QPointF> DataLoader::convertToQPointFList(const std::vector<std::pair<double, double>>& data) {
    QList<QPointF> result;
    for (const auto& point : data) {
        // 使用 QPointF 构造函数直接从 double 类型创建对象
        result.append(QPointF(point.first, point.second));
    }
    return result;
}

Eigen::MatrixXd DataLoader::convertToEigenMatrix(const std::vector<std::pair<double, double>> &data) {
    Eigen::MatrixXd matrix(data.size(), 2); // 每行有两个字段
    for (size_t i = 0; i < data.size(); ++i) {
        matrix(i, 0) = data[i].first;  // x
        matrix(i, 1) = data[i].second; // y
    }
    return matrix;
}

