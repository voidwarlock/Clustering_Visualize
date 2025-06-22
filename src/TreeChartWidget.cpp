// TreeChartWidget.cpp
#include "TreeChartWidget.h"
#include <QPainter>
#include <QFontMetrics>
#include <algorithm>
#include <stack>
#include "Label2Color.h"

TreeChartWidget::TreeChartWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(600); // Set minimum height
}

void TreeChartWidget::initColorBiases(int n_clusters) {
    colorBiases_.clear();
    if (n_clusters <= 0) return;

    double step = PERHEIGHT / n_clusters;
    double start = -PERHEIGHT / 2.0 + step / 2.0;

    for (int i = 0; i < n_clusters; ++i) {
        colorBiases_.push_back(start + i * step);
    }
}

void TreeChartWidget::setData(const std::vector<ClusterNode*>& roots, const QList<QPointF>& points, const std::vector<int>& labels, int maxheight, int nclusters) {
    roots_ = roots;
    points_ = points;
    labels_ = labels;
    max_height = maxheight;
    initColorBiases(nclusters);
    update(); // Trigger a repaint when new data is set
}

void TreeChartWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    if (roots_.empty()) {
        return; // Nothing to draw
    }

    painter.fillRect(rect(), Qt::white);
    
    int chartHeight = max_height * PERHEIGHT;
    int chartWidth = PERWIDTH * (points_.size() + 30);

    setMinimumWidth(chartWidth + 2*MARGIN);
    setMinimumHeight(chartHeight + 2*MARGIN);

    QColor color = Qt::black;
    QPen pen(color);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawLine(MARGIN, MARGIN + chartHeight, MARGIN + chartWidth, MARGIN + chartHeight);
    painter.drawLine(MARGIN, MARGIN, MARGIN, MARGIN + chartHeight);

    // Y 轴刻度
    for (int i = 0; i <= max_height; ++i) {
        int yPos = MARGIN + chartHeight - i * PERHEIGHT;
        painter.drawLine(MARGIN - 5, yPos, MARGIN, yPos);
        QString label = QString::number(i);
        painter.drawText(MARGIN - 40, yPos + 4, label);
    }

    painter.drawText(MARGIN - 30, MARGIN + chartHeight + 32, "X:");
    painter.drawText(MARGIN - 30, MARGIN + chartHeight + 44, "Y:");
    QFontMetrics fm(painter.font());
    // X 轴刻度
    for(int i=0; i<points_.size(); i++){
        int x = MARGIN + (i+1) * PERWIDTH - 10;
        
        QString labelText = QString("P%1").arg(i);
        painter.drawText(x, MARGIN + chartHeight + 15, labelText);

        // 绘制points_的数据，显示为 (x, y) 形式，并换行显示
        if (i < points_.size()) {
            QPointF point = points_[i];
            QString xpointLabel = QString("%1").arg(point.x(), 0, 'f', 2);
            int xtextWidth = fm.horizontalAdvance(xpointLabel);
            painter.drawText(x, MARGIN + chartHeight + 32, xpointLabel);
            QString ypointLabel = QString("%1").arg(point.y(), 0, 'f', 2);
            int ytextWidth = fm.horizontalAdvance(ypointLabel);
            painter.drawText(x, MARGIN + chartHeight + 44, ypointLabel);       
        }
    }

    for (auto root : roots_){
        drawNode(painter, root, chartHeight);
    }
    
}

std::pair<int, int> TreeChartWidget::drawNode(QPainter& painter, ClusterNode* node, int chartHeight) {
    int label;
    int startx;
    if (node->left == nullptr && node->right == nullptr){
        if(node->id<labels_.size()){
            label = labels_[node->id];
        }
        startx = (node->id + 1) * PERWIDTH + MARGIN;

        return std::make_pair(startx, label);
    } 

    std::pair<int, int>left_node = drawNode(painter, node->left, chartHeight);
    std::pair<int, int>right_node = drawNode(painter, node->right, chartHeight);

    startx = (left_node.first + right_node.first) / 2;
    int bias;
    if(left_node.second == right_node.second && left_node.second!=-2 && right_node.second!=-2){
        label = left_node.second;
        bias = MARGIN+chartHeight-PERHEIGHT*node->height+colorBiases_[label];
    }
    else {
        label = -2;
        bias = MARGIN+chartHeight-PERHEIGHT*node->height;
    }

    QColor color = getColorForLabel(label);
    QPen pen(color);
    pen.setWidth(2);
    painter.setPen(pen);

    int left_bias = MARGIN+chartHeight-PERHEIGHT*node->left->height;
    int right_bias = MARGIN+chartHeight-PERHEIGHT*node->right->height;
    if(node->left->id>=labels_.size()){
        left_bias+=colorBiases_[left_node.second];
    }
    if(node->right->id>=labels_.size()){
        right_bias+=colorBiases_[right_node.second];
    }

    painter.drawLine(left_node.first, left_bias, left_node.first, bias);
    painter.drawLine(right_node.first, right_bias, right_node.first, bias);

    painter.drawLine(left_node.first, bias, right_node.first, bias);

    return std::make_pair(startx, label);
}
