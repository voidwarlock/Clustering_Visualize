// DataLoader.cpp
#include "LoadData.h"       // 本文件对应的头文件
#include <fstream>          // 用于文件输入输出（std::ifstream）
#include <sstream>          // 用于字符串流解析（std::istringstream）
#include <iostream>         // 用于标准输入输出（std::cerr）
#include <vector>           // 使用 std::vector 存储数据点
#include <Eigen/Dense>      // Eigen 库，用于矩阵转换

/**
 * 构造函数：初始化数据加载器
 * @param filename 输入数据文件路径
 */
DataLoader::DataLoader(const std::string &filename)
    : filename_(filename) {}

/**
 * 从文件中加载二维数据点（x, y）组成的列表
 * @return 包含数据点的 vector，每个元素是一个 (x, y) 的 pair
 */
std::vector<std::pair<double, double>> DataLoader::load() {
    std::ifstream file(filename_); // 打开指定文件
    std::vector<std::pair<double, double>> dataPoints; // 存储读取的数据点

    if (!file.is_open()) {
        // 如果文件打开失败，输出错误信息
        std::cerr << "无法打开文件: " << filename_ << std::endl;
        return dataPoints; // 返回空 vector
    }

    std::string line;
    while (std::getline(file, line)) { // 按行读取文件内容
        std::istringstream ss(line);   // 将一行数据转为字符串流
        double x, y;

        // 从当前行提取两个 double 值作为 x 和 y
        if (ss >> x >> y) {
            dataPoints.emplace_back(x, y); // 添加到数据容器
        } else {
            std::cerr << "数据解析失败: " << line << std::endl;
        }
    }

    file.close(); // 关闭文件
    return dataPoints;
}

/**
 * 将数据点转换为 QList<QPointF> 类型，便于 Qt 图形界面使用
 * @param data 数据点集合（std::vector<std::pair<double, double>>）
 * @return 转换后的 QPointF 列表
 */
QList<QPointF> DataLoader::convertToQPointFList(
    const std::vector<std::pair<double, double>>& data) {
    
    QList<QPointF> result;

    for (const auto& point : data) {
        // 将每个 pair<double, double> 转换为 QPointF 对象并加入列表
        result.append(QPointF(point.first, point.second));
    }

    return result;
}

/**
 * 将数据点转换为 Eigen::MatrixXd 矩阵类型，便于数值计算使用
 * 每个数据点对应矩阵的一行，包含两列（x 和 y）
 * @param data 数据点集合
 * @return 转换后的 Eigen 矩阵（n x 2）
 */
Eigen::MatrixXd DataLoader::convertToEigenMatrix(
    const std::vector<std::pair<double, double>> &data) {
    
    // 创建一个 n 行 2 列的矩阵
    Eigen::MatrixXd matrix(data.size(), 2);

    for (size_t i = 0; i < data.size(); ++i) {
        matrix(i, 0) = data[i].first;   // 第一列为 x 值
        matrix(i, 1) = data[i].second;  // 第二列为 y 值
    }

    return matrix;
}