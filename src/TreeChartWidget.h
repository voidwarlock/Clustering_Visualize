#ifndef TREECHARTWIDGET_H
#define TREECHARTWIDGET_H

#pragma once  // 确保该头文件只被包含一次（可选，与下面的 #ifndef 配合使用）

#include <QWidget>          // Qt 基础控件类
#include <vector>           // 使用 vector 存储聚类节点和标签数据
#include <QList>            // 使用 QList 存储 QPointF 类型的数据点
#include <QPoint>           // 用于表示二维坐标点（虽然主要是 QPointF 在用）
#include "clustering/Cluster.h"  // 包含 ClusterNode 定义（层次聚类树的节点结构）

// 图表边距常量定义
#define MARGIN    50     // 图表四周留白边距（像素）
#define PERHEIGHT 100    // 每层的高度（用于 Y 轴布局）
#define PERWIDTH   45    // 每个点在 X 轴上的宽度（用于 X 轴布局）

/**
 * @class TreeChartWidget
 * @brief 自定义控件，用于绘制层次聚类树（Agglomerative Clustering Tree）。
 *
 * 该控件继承自 QWidget，能够可视化显示聚类过程中生成的树状结构，
 * 支持动态设置数据、递归绘制树形连接线，并根据聚类类别进行颜色区分。
 */
class TreeChartWidget : public QWidget {
    Q_OBJECT  // 启用 Qt 的信号与槽机制（虽然当前未使用，但保留以备扩展）

public:
    /**
     * 构造函数：创建一个用于绘制聚类树的控件
     * @param parent 父级 QWidget，默认为 nullptr
     */
    explicit TreeChartWidget(QWidget *parent = nullptr);

    /**
     * 设置要绘制的聚类树数据
     *
     * @param roots 层次聚类树的根节点列表（每个根代表一棵树）
     * @param points 数据点坐标列表（用于标注或绘图）
     * @param labels 每个点对应的类别标签（用于颜色映射）
     * @param maxheight 树的最大高度（用于图表布局计算）
     * @param nclusters 目标聚类数量（用于颜色区分）
     */
    void setData(const std::vector<ClusterNode*> &roots,
                 const QList<QPointF> &points,
                 const std::vector<int> &labels,
                 int maxheight,
                 int nclusters);

    /**
     * 初始化颜色偏移值，用于多聚类层级的颜色区分
     * @param n_clusters 聚类数量
     */
    void initColorBiases(int n_clusters);

    /**
     * 递归绘制指定节点及其子节点
     *
     * @param painter QPainter 对象，用于绘图
     * @param node 当前绘制的节点
     * @param chartHeight 图表总高度
     * @return 返回当前节点在 X 轴上的位置及对应标签
     */
    std::pair<int, int> drawNode(QPainter& painter, ClusterNode* node, int chartHeight);

protected:
    /**
     * 绘图事件处理函数，负责整个树状图的绘制
     * @param event 绘图事件参数
     */
    void paintEvent(QPaintEvent *event) override;

private:
    int max_height;                    ///< 树的最大高度，用于布局计算
    std::vector<ClusterNode*> roots_; ///< 聚类树的根节点列表
    QList<QPointF> points_;           ///< 数据点坐标列表
    std::vector<int> labels_;         ///< 每个点对应的类别标签
    std::vector<double> colorBiases_; ///< 颜色偏移数组，用于不同聚类层级颜色区分
};

#endif // TREECHARTWIDGET_H