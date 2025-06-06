#include "ShowTree.h"
#include <QFontMetrics>
#include <algorithm>

SubWindowTree::SubWindowTree(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Agglomerative Tree Chart");
    scrollArea_ = new QScrollArea(this);
    chartWidget_ = new TreeChartWidget(scrollArea_);
    scrollArea_->setWidget(chartWidget_);
    scrollArea_->setWidgetResizable(false); // 不自动缩放内部 widget
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(scrollArea_);
    setLayout(layout);
}

void SubWindowTree::setData(const std::vector<ClusterNode*> &roots, const QList<QPointF> &points, const std::vector<int> &labels, int nclusters) {
    int max_height = 0;
    for (auto root : roots) {
        if(root->height > max_height){
            max_height = root->height;
        }
    }
    chartWidget_->setData(roots, points, labels, max_height, nclusters);
    chartWidget_->resize(PERWIDTH * points.size(), PERHEIGHT * max_height); 
}