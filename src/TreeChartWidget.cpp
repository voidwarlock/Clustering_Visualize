// TreeChartWidget.cpp
#include "TreeChartWidget.h"   // 自定义树状图控件头文件
#include <QPainter>            // 用于绘图操作
#include <QFontMetrics>        // 用于文本宽度计算
#include <algorithm>           // 算法库（未直接使用，但可能用于后续扩展）
#include <stack>               // 可能用于非递归遍历（当前未用到）
#include "Label2Color.h"       // 标签映射颜色工具

/**
 * @brief 构造函数：初始化树状图控件
 * @param parent 父级 QWidget，默认为 nullptr
 */
TreeChartWidget::TreeChartWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(600); // 设置控件最小高度
}

/**
 * @brief 初始化颜色偏移值，用于多聚类层级的颜色区分
 * @param n_clusters 聚类数量
 */
void TreeChartWidget::initColorBiases(int n_clusters) {
    colorBiases_.clear();
    if (n_clusters <= 0) return;

    // 每个聚类在 Y 轴方向分配一个偏移量，使得同一类别节点颜色一致
    double step = PERHEIGHT / n_clusters;
    double start = -PERHEIGHT / 2.0 + step / 2.0;

    for (int i = 0; i < n_clusters; ++i) {
        colorBiases_.push_back(start + i * step);
    }
}

/**
 * @brief 设置树状图所需的数据
 *
 * @param roots 层次聚类树的根节点列表
 * @param points 数据点坐标列表
 * @param labels 每个点对应的标签（用于颜色映射）
 * @param maxheight 树的最大高度（用于布局计算）
 * @param nclusters 目标聚类数量（用于颜色区分）
 */
void TreeChartWidget::setData(const std::vector<ClusterNode*>& roots,
                              const QList<QPointF>& points,
                              const std::vector<int>& labels,
                              int maxheight,
                              int nclusters)
{
    roots_ = roots;
    points_ = points;
    labels_ = labels;
    max_height = maxheight;

    initColorBiases(nclusters); // 初始化颜色偏移
    update(); // 触发重绘事件
}

/**
 * @brief 绘图事件处理函数，负责绘制整个层次聚类树
 * @param QPaintEvent* 事件参数（未使用）
 */
void TreeChartWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 启用抗锯齿

    if (roots_.empty()) {
        return; // 如果没有数据，不进行绘制
    }

    painter.fillRect(rect(), Qt::white); // 填充背景为白色

    int chartHeight = max_height * PERHEIGHT; // 图表总高度
    int chartWidth = PERWIDTH * (points_.size() + 30); // 图表总宽度

    // 设置控件最小尺寸以适应图表内容
    setMinimumWidth(chartWidth + 2 * MARGIN);
    setMinimumHeight(chartHeight + 2 * MARGIN);

    // 绘制坐标轴
    QColor axisColor = Qt::black;
    QPen pen(axisColor);
    pen.setWidth(2);
    painter.setPen(pen);

    painter.drawLine(MARGIN, MARGIN + chartHeight, MARGIN + chartWidth, MARGIN + chartHeight); // X轴
    painter.drawLine(MARGIN, MARGIN, MARGIN, MARGIN + chartHeight); // Y轴

    // Y轴刻度与标签
    for (int i = 0; i <= max_height; ++i) {
        int yPos = MARGIN + chartHeight - i * PERHEIGHT;
        painter.drawLine(MARGIN - 5, yPos, MARGIN, yPos);
        QString label = QString::number(i);
        painter.drawText(MARGIN - 40, yPos + 4, label);
    }

    // X轴标签说明
    painter.drawText(MARGIN - 30, MARGIN + chartHeight + 32, "X:");
    painter.drawText(MARGIN - 30, MARGIN + chartHeight + 44, "Y:");

    QFontMetrics fm(painter.font());

    // X轴数据点标注
    for (int i = 0; i < points_.size(); i++) {
        int x = MARGIN + (i + 1) * PERWIDTH - 10;

        QString labelText = QString("P%1").arg(i); // 点编号
        painter.drawText(x, MARGIN + chartHeight + 15, labelText);

        // 显示每个点的坐标 (x, y)
        if (i < points_.size()) {
            QPointF point = points_[i];
            QString xpointLabel = QString("%1").arg(point.x(), 0, 'f', 2); // 保留两位小数
            int xtextWidth = fm.horizontalAdvance(xpointLabel);
            painter.drawText(x, MARGIN + chartHeight + 32, xpointLabel);

            QString ypointLabel = QString("%1").arg(point.y(), 0, 'f', 2);
            painter.drawText(x, MARGIN + chartHeight + 44, ypointLabel);
        }
    }

    // 遍历所有根节点并绘制树结构
    for (auto root : roots_) {
        drawNode(painter, root, chartHeight);
    }
}

/**
 * @brief 递归绘制树节点及其连接线
 * @param painter QPainter 对象
 * @param node 当前节点指针
 * @param chartHeight 图表总高度
 * @return 返回当前节点的 X 坐标和对应标签
 */
std::pair<int, int> TreeChartWidget::drawNode(QPainter& painter, ClusterNode* node, int chartHeight)
{
    int label;
    int startx;

    // 如果是叶子节点
    if (node->left == nullptr && node->right == nullptr) {
        if (node->id < static_cast<int>(labels_.size())) {
            label = labels_[node->id]; // 获取标签
        } else {
            label = -2; // 无效标签
        }

        // 计算该点在 X 轴上的位置
        startx = (node->id + 1) * PERWIDTH + MARGIN;

        return std::make_pair(startx, label);
    }

    // 递归绘制左右子节点
    std::pair<int, int> left_node = drawNode(painter, node->left, chartHeight);
    std::pair<int, int> right_node = drawNode(painter, node->right, chartHeight);

    startx = (left_node.first + right_node.first) / 2; // 当前节点居中于两个子节点之间

    // 判断左右子节点是否属于同一类别
    if (left_node.second == right_node.second &&
        left_node.second != -2 &&
        right_node.second != -2) {
        label = left_node.second;
    } else {
        label = -2; // 不同类别或无效
    }

    // 根据标签获取对应颜色
    QColor color = getColorForLabel(label);
    QPen pen(color);
    pen.setWidth(2);
    painter.setPen(pen);

    // 获取左右子节点 Y 方向偏移
    int left_bias = MARGIN + chartHeight - PERHEIGHT * node->left->height;
    int right_bias = MARGIN + chartHeight - PERHEIGHT * node->right->height;

    // 如果子节点不是叶子，则加上颜色偏移
    if (node->left->id >= static_cast<int>(labels_.size())) {
        left_bias += colorBiases_[left_node.second];
    }
    if (node->right->id >= static_cast<int>(labels_.size())) {
        right_bias += colorBiases_[right_node.second];
    }

    // 绘制连接线
    painter.drawLine(left_node.first, left_bias, left_node.first, MARGIN + chartHeight - PERHEIGHT * node->height);
    painter.drawLine(right_node.first, right_bias, right_node.first, MARGIN + chartHeight - PERHEIGHT * node->height);
    painter.drawLine(left_node.first, MARGIN + chartHeight - PERHEIGHT * node->height,
                     right_node.first, MARGIN + chartHeight - PERHEIGHT * node->height);

    // 返回当前节点的 X 坐标和标签
    return std::make_pair(startx, label);
}