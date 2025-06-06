// LoadData.h
#ifndef LOADDATA_H
#define LOADDATA_H

#include <vector>
#include <string>
#include <utility> // std::pair
#include <QList>
#include <QPoint>
#include <Eigen/Dense>

class DataLoader {
public:
    // 构造函数接受文件名
    explicit DataLoader(const std::string &filename);

    // 加载数据，返回前两个字段组成的 vector<pair<double, double>>
    std::vector<std::pair<double, double>> load();

    QList<QPointF> convertToQPointFList(const std::vector<std::pair<double, double>>& data);
    Eigen::MatrixXd convertToEigenMatrix(const std::vector<std::pair<double, double>> &data);
private:
    std::string filename_;
};

#endif // DATALOADER_H