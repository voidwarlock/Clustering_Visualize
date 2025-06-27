#include "ShowTree.h"       // 对应的头文件声明
#include <QFontMetrics>     // 字体度量（可能用于文本绘制计算）
#include <algorithm>        // 算法库（如排序、查找等，当前未直接使用）

/**
 * @brief 构造函数：创建一个显示层次聚类树的子窗口
 * @param parent 父级 QWidget，默认为 nullptr
 */
SubWindowTree::SubWindowTree(QWidget *parent)
    : QDialog(parent) // 继承自 QDialog，作为模态或非模态对话框显示
{
    // 设置窗口标题
    setWindowTitle("Agglomerative Tree Chart");

    // 创建滚动区域，以便在图表过大时支持横向和纵向滚动查看
    scrollArea_ = new QScrollArea(this);

    // 创建树状图控件，并将其嵌入滚动区域中
    chartWidget_ = new TreeChartWidget(scrollArea_);

    // 将树状图控件设置为滚动区域的内部部件
    scrollArea_->setWidget(chartWidget_);

    // 设置不自动调整 widget 大小，保持 chartWidget_ 的原始尺寸
    scrollArea_->setWidgetResizable(false);

    // 横向和纵向滚动条按需显示
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 创建主布局并添加滚动区域
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(scrollArea_);
    setLayout(layout);
}

/**
 * @brief 设置子窗口中要显示的层次聚类树数据
 *
 * 该方法将传入的聚类树根节点、点坐标、标签及目标聚类数传递给底层的树状图控件，
 * 并根据树的最大高度和点的数量动态调整图表大小以适应滚动查看。
 *
 * @param roots 层次聚类树的根节点列表（每个根代表一棵树）
 * @param points 数据点坐标列表（用于标注或绘图）
 * @param labels 每个点对应的类别标签（用于颜色映射）
 * @param nclusters 目标聚类数量（用于可视化分层切割）
 */
void SubWindowTree::setData(const std::vector<ClusterNode*> &roots,
                             const QList<QPointF> &points,
                             const std::vector<int> &labels,
                             int nclusters)
{
    // 计算所有树中的最大高度，用于确定图表的垂直空间
    int max_height = 0;
    for (auto root : roots) {
        if (root->height > max_height) {
            max_height = root->height;
        }
    }

    // 将数据传递给树状图控件进行显示
    chartWidget_->setData(roots, points, labels, max_height, nclusters);

    // 动态调整树状图控件的大小：
    // 宽度 = 每个点的宽度（PERWIDTH） × 点的数量；
    // 高度 = 每层的高度（PERHEIGHT） × 最大层数（max_height）；
    chartWidget_->resize(PERWIDTH * points.size(), PERHEIGHT * max_height);
}