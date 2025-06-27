#ifndef COORDINATEWIDGET_H
#define COORDINATEWIDGET_H

#include <QWidget>                  // 基类 QWidget
#include <QList>                    // 使用 QList 容器
#include <QPoint>                   // 用于鼠标坐标记录
#include <QPointF>                  // 表示二维点 (x, y)
#include <QWheelEvent>              // 鼠标滚轮事件
#include <QSlider>                  // 缩放滑块控件
#include <iostream>                 // 标准输入输出（可能用于调试）
#include <vector>                   // 使用 std::vector 存储数据结构

// 自定义头文件
#include "clustering/Cluster.h"     // 聚类相关类（如 ClusterNode）
#include "ShowProbs.h"              // 显示概率窗口
#include "ShowTree.h"               // 显示树状结构窗口

/**
 * @class CoordinateWidget
 * @brief 自定义绘图控件，用于在二维坐标系中绘制点、聚类结果、辅助信息等。
 *
 * 支持：
 * - 缩放和平移操作；
 * - 绘制网格、坐标轴、点、核心点、标记；
 * - 接收外部数据更新并重绘；
 * - 弹出子窗口显示详细信息（如概率分布或树结构）；
 * - 鼠标交互：拖动、缩放。
 */
class CoordinateWidget : public QWidget {
    Q_OBJECT // Qt 的信号和槽机制支持

public:
    /**
     * 构造函数
     * @param parent 父级 widget，默认为 nullptr
     */
    explicit CoordinateWidget(QWidget *parent = nullptr);

    /**
     * 获取当前绘制的点集
     * @return 当前点列表
     */
    const QList<QPointF>& getPoints() const;

    /**
     * 设置要绘制的点集
     * @param points 新的点列表
     */
    void setPoints(const QList<QPointF> &points);

    /**
     * 设置缩放比例
     * @param scale 缩放因子
     */
    void setScale(qreal scale);

    /**
     * 获取当前缩放比例
     * @return 缩放因子
     */
    qreal getScale() const;

public slots:

    /**
     * 设置点的标签（用于颜色映射）
     * @param w_labels 标签数组
     */
    void setLabels(const std::vector<int>& w_labels);

    /**
     * 设置聚类中心
     * @param w_centers 聚类中心数组
     */
    void setCenters(const std::vector<std::vector<double>>& w_centers);

    /**
     * 设置点的特征信息（例如 DBSCAN 中是否为核心点等）
     * @param w_point_features 特征数组
     * @param w_eps 用于判断的核心半径
     */
    void setPoint_features(const std::vector<Pointtype>& w_point_features, double w_eps);

    /**
     * 设置每个点的概率值（可能用于可视化）
     * @param w_probs 概率数组
     */
    void setProbs(const std::vector<double>& w_probs);

    /**
     * 设置聚类树结构（如层次聚类）
     * @param w_roots 聚类根节点指针列表
     * @param nClusters 聚类数量
     */
    void setRoots(const std::vector<ClusterNode*>& w_roots, int nClusters);

    /**
     * 控制是否启用辅助图形（如概率窗、树结构窗）
     * @param flag 开关标志
     */
    void setFlags(bool flag);

signals:

    /**
     * 当缩放比例改变时发出此信号
     * @param scale 新的缩放比例
     */
    void scaleChanged(qreal scale);

    /**
     * 当点集发生改变时发出此信号
     * @param points 新的点列表
     */
    void pointsChanged(const QList<QPointF> &points);

protected:

    /**
     * 绘图事件处理函数
     * 重写以自定义绘图逻辑
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * 鼠标按下事件处理函数
     * 用于开始拖动视图
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * 鼠标移动事件处理函数
     * 用于拖动视图
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * 鼠标释放事件处理函数
     * 用于结束拖动
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * 鼠标滚轮事件处理函数
     * 用于调整缩放比例
     */
    void wheelEvent(QWheelEvent *event) override;

private slots:

    /**
     * 缩放滑块变化响应函数
     * 将滑块值转换为实际缩放比例
     */
    void onScaleSliderChanged(int value);

private:

    /**
     * 绘制背景网格线
     */
    void drawGrid(QPainter *painter);

    /**
     * 绘制坐标轴
     */
    void drawAxes(QPainter *painter);

    /**
     * 绘制带标签的圆形点（用于聚类展示）
     */
    void drawCircle(QPainter *painter, const QPointF &point, int label);

    /**
     * 绘制核心点（如 DBSCAN 中的核心点）
     */
    void drawCore(QPainter *painter, const QPointF &point, int label);

    /**
     * 绘制交叉点（用于特殊标记）
     */
    void drawCross(QPainter *painter, const QPointF &point);

    /**
     * 绘制图例（颜色与标签对应关系）
     */
    void drawLegend(QPainter *painter);

    // 数据成员
    QList<QPointF> points;          ///< 当前绘制的点列表
    qreal scale = 1.0;              ///< 当前缩放比例
    QPointF offset;                 ///< 视图偏移量（平移）
    QPoint lastMousePosition;       ///< 上一次鼠标位置，用于拖动计算
    bool dragging = false;          ///< 是否正在拖动
    int Pointscale = 50;            ///< 点的大小比例系数（可能用于绘制）
    QSlider *scaleSlider = nullptr;///< 缩放控制滑块指针

    SubWindowProbs *prob_window;    ///< 概率展示子窗口
    SubWindowTree *tree_window;     ///< 树结构展示子窗口

    bool drawAuxi = false;          ///< 是否绘制辅助信息（如概率窗口触发）

    std::vector<int> labels;        ///< 每个点对应的标签
    std::vector<std::vector<double>> centers; ///< 聚类中心坐标
    std::vector<Pointtype> point_features; ///< 点的类型特征（如是否为核心点）
    double eps;                     ///< 核心区域半径参数
};

#endif // COORDINATEWIDGET_H