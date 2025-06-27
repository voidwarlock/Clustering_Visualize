#ifndef SHOWTREE_H
#define SHOWTREE_H

// Qt 标准库头文件
#include <QDialog>         // 用于创建模态或非模态对话框窗口
#include <QVBoxLayout>     // 垂直布局管理器
#include <QScrollArea>     // 提供滚动区域支持，便于查看大图表

// C++ 标准库头文件
#include <vector>          // 使用 vector 存储节点和标签数据

// 自定义控件头文件
#include "TreeChartWidget.h" // 树状图绘制控件（需自行实现）

// 前向声明 ClusterNode 结构体/类（避免包含整个头文件）
class ClusterNode;

/**
 * @class SubWindowTree
 * @brief 显示层次聚类树结构的子窗口类。
 *
 * 该类继承自 QDialog，用于在独立窗口中展示 Agglomerative Clustering 的树形结构。
 * 内部使用 TreeChartWidget 绘制树状图，并通过 QScrollArea 实现滚动查看，
 * 支持可视化大规模聚类树。
 */
class SubWindowTree : public QDialog {
    Q_OBJECT // 启用 Qt 的信号与槽机制

public:
    /**
     * 构造函数：初始化子窗口界面
     * @param parent 父级 QWidget，默认为 nullptr
     */
    explicit SubWindowTree(QWidget *parent = nullptr);

    /**
     * 设置要显示的层次聚类树数据及其相关点坐标和标签
     *
     * @param roots 层次聚类树的根节点列表（每个根代表一棵树）
     * @param points 数据点坐标列表（用于标注或绘图）
     * @param labels 每个点对应的类别标签（用于颜色映射）
     * @param nclusters 目标聚类数量（用于在树上标记切割层级）
     */
    void setData(const std::vector<ClusterNode*> &roots,
                 const QList<QPointF> &points,
                 const std::vector<int> &labels,
                 int nclusters);

private:
    QScrollArea *scrollArea_;      ///< 滚动区域，用于容纳树状图并支持滚动查看
    TreeChartWidget *chartWidget_; ///< 实际用于绘制层次聚类树的自定义控件
};

#endif // SHOWTREE_H