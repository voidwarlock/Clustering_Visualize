#ifndef LOADDATA_H
#define LOADDATA_H

// 标准库头文件
#include <vector>       // 使用 std::vector 存储数据点
#include <string>       // 使用 std::string 表示文件路径
#include <utility>      // 使用 std::pair 来表示二维数据点 (x, y)

// Qt 头文件
#include <QList>        // 使用 QList 类型用于 Qt 界面交互
#include <QPointF>      // 使用 QPointF 表示二维点（Qt 中常用）

// Eigen 数学库
#include <Eigen/Dense>  // 使用 Eigen 库进行矩阵运算

/**
 * @class DataLoader
 * @brief 数据加载器类，用于从文本文件中读取二维数据点，并提供多种格式转换功能。
 *
 * 支持：
 * - 从文件中读取每行两个 double 值组成的数据；
 * - 转换为 Qt 图形界面可用的 QList<QPointF>；
 * - 转换为 Eigen 可用的 MatrixXd 矩阵（适合数值计算）；
 */
class DataLoader {
public:
    /**
     * 构造函数：初始化数据加载器
     * @param filename 数据文件路径（CSV 或 TXT 格式）
     */
    explicit DataLoader(const std::string &filename);

    /**
     * 加载文件中的二维数据点
     * @return 包含 (x, y) 数据点的 vector<pair<double, double>>
     */
    std::vector<std::pair<double, double>> load();

    /**
     * 将数据点转换为 Qt 可用的 QList<QPointF> 格式
     * @param data 输入的 pair<double, double> 数据点列表
     * @return 转换后的 QList<QPointF>
     */
    QList<QPointF> convertToQPointFList(const std::vector<std::pair<double, double>>& data);

    /**
     * 将数据点转换为 Eigen 的 MatrixXd 格式（n × 2 矩阵）
     * @param data 输入的 pair<double, double> 数据点列表
     * @return Eigen::MatrixXd（n × 2），用于数值计算或机器学习算法
     */
    Eigen::MatrixXd convertToEigenMatrix(const std::vector<std::pair<double, double>> &data);

private:
    std::string filename_; ///< 要加载的数据文件名
};

#endif // LOADDATA_H