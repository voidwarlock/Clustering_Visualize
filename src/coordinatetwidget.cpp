#include "coordinatetwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <algorithm>
#include <iostream>
#include <QVBoxLayout>
#include "Label2Color.h"

CoordinateWidget::CoordinateWidget(QWidget *parent)
    : QWidget(parent), prob_window(nullptr), tree_window(nullptr){
        // 创建滑动条
    scaleSlider = new QSlider(Qt::Horizontal, this);
    scaleSlider->setMinimum(20);
    scaleSlider->setMaximum(400);
    scaleSlider->setValue(Pointscale);

    // 设置布局管理器将滑动条放在底部
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch();
    mainLayout->addWidget(scaleSlider);

    // 连接信号和槽
    connect(scaleSlider, &QSlider::valueChanged, this, &CoordinateWidget::onScaleSliderChanged);
}

void CoordinateWidget::onScaleSliderChanged(int value)
{
    Pointscale = value;
    update();  // 触发重绘
}

void CoordinateWidget::setScale(qreal newScale) {
    scale = qBound(0.1, newScale, 10.0);
    update();
}

qreal CoordinateWidget::getScale() const {
    return scale;
}

const QList<QPointF>& CoordinateWidget::getPoints() const {
    return points;
}

void CoordinateWidget::setPoints(const QList<QPointF> &newPoints) {
    if (points != newPoints) {
        points = newPoints;
        labels = std::vector<int>(points.size(),-1);
        update(); // 触发重绘
        emit pointsChanged(points); // 发出信号
    }
}

void CoordinateWidget::setLabels(const std::vector<int>& w_labels){
    labels = w_labels;
    update();
}

void CoordinateWidget::setCenters(const std::vector<std::vector<double>>& w_centers){
    centers = w_centers;
    update();
}

void CoordinateWidget::setPoint_features(const std::vector<Pointtype>& w_point_features, double w_eps){
    point_features = w_point_features;
    eps = w_eps;
    update();
}

void CoordinateWidget::setProbs(const std::vector<double>& probs) {
    if (!probs.empty() && drawAuxi) {
        if (points.isEmpty()) {
            std::cerr << "Points list is empty!" << std::endl;
            return;
        }
        if (!prob_window) {
            prob_window = new SubWindowProbs(this);
            prob_window->resize(800, 600);
            prob_window->move(600, 100);
            prob_window->show();
            // 连接 destroyed 信号，在窗口关闭时将指针置空
            connect(prob_window, &QObject::destroyed, this, [this]() {
                prob_window = nullptr;
            });
        }
        prob_window->setData(probs, points, labels); 

    } else {
        if (prob_window) {
            delete prob_window; // 会触发 destroyed 信号，自动置空
            prob_window = nullptr;
        }
    }
}


void CoordinateWidget::setRoots(const std::vector<ClusterNode*>& roots, int nClusters){
    if (!roots.empty() && drawAuxi) {
        if (points.isEmpty()) {
            std::cerr << "Points list is empty!" << std::endl;
            return;
        }
        if (!tree_window) {
            tree_window = new SubWindowTree(this);
            tree_window->resize(800, 600);
            tree_window->move(600, 100);
            tree_window->show();
            connect(tree_window, &QObject::destroyed, this, [this]() {
                tree_window = nullptr;
            });
        }
        tree_window->setData(roots, points, labels, nClusters); 
        
    } else {
        if (tree_window) {
            delete tree_window; // 会触发 destroyed 信号，自动置空
            tree_window = nullptr;
        }
    }
}

void CoordinateWidget::setFlags(bool flag){
    if(flag){
        drawAuxi = !drawAuxi;
    }
    else{
        drawAuxi = false;
    }
    update();
}

void CoordinateWidget::drawGrid(QPainter *painter) {
    painter->save();
    painter->setPen(Qt::lightGray);

    // 计算当前视图范围（逻辑坐标）
    qreal halfWidth = width() / (2 * scale);
    qreal halfHeight = height() / (2 * scale);

    // 定义足够大的延伸范围（可根据需要调整这个倍数）
    const qreal extensionFactor = 1000.0;
    qreal extendedWidth = halfWidth * extensionFactor;
    qreal extendedHeight = halfHeight * extensionFactor;

    // 绘制无限延伸的网格线
    int startX = static_cast<int>((-halfWidth - offset.x()/scale) / Pointscale) * Pointscale;
    int endX = static_cast<int>((halfWidth - offset.x()/scale) / Pointscale) * Pointscale;
    int startY = static_cast<int>((-halfHeight - offset.y()/scale) / Pointscale) * Pointscale;
    int endY = static_cast<int>((halfHeight - offset.y()/scale) / Pointscale) * Pointscale;

    // 扩展一些网格线以确保平滑过渡
    startX -= Pointscale * 2;
    endX += Pointscale * 2;
    startY -= Pointscale * 2;
    endY += Pointscale * 2;

    for (int x = startX; x <= endX; x += Pointscale) {
        painter->drawLine(x, startY, x, endY);
    }

    for (int y = startY; y <= endY; y += Pointscale) {
        painter->drawLine(startX, y, endX, y);
    }

    painter->restore();
}


void CoordinateWidget::drawAxes(QPainter *painter) {
    painter->save();
    
    // 设置字体和画笔
    int fontSize = qBound(8, static_cast<int>(10 / scale), 20);
    QFont font = painter->font();
    font.setPointSize(fontSize);
    painter->setFont(font);
    painter->setPen(Qt::black);

    // 计算当前视图范围（逻辑坐标）
    qreal halfWidth = width() / (2 * scale);
    qreal halfHeight = height() / (2 * scale);
    
    // 定义足够大的延伸范围（可根据需要调整这个倍数）
    const qreal extensionFactor = 1000.0;
    qreal extendedWidth = halfWidth * extensionFactor;
    qreal extendedHeight = halfHeight * extensionFactor;

    // 绘制无限延伸的坐标轴
    painter->drawLine(-extendedWidth, 0, extendedWidth, 0); // X轴
    painter->drawLine(0, -extendedHeight, 0, extendedHeight); // Y轴

    // 网格和刻度参数
    const int tickLength = 5;

    // 计算可见区域内的网格线范围
    int startX = static_cast<int>((-halfWidth - offset.x()/scale) / Pointscale) * Pointscale;
    int endX = static_cast<int>((halfWidth - offset.x()/scale) / Pointscale) * Pointscale;
    int startY = static_cast<int>((-halfHeight - offset.y()/scale) / Pointscale) * Pointscale;
    int endY = static_cast<int>((halfHeight - offset.y()/scale) / Pointscale) * Pointscale;

    // 扩展一些网格线以确保平滑过渡
    startX -= Pointscale * 2;
    endX += Pointscale * 2;
    startY -= Pointscale * 2;
    endY += Pointscale * 2;

    QRectF visibleRect(
        -halfWidth - offset.x()/scale,
        -halfHeight - offset.y()/scale,
        width() / scale,
        height() / scale
    );
    // 绘制X轴刻度
    for (int x = startX; x <= endX; x += Pointscale) {
        painter->drawLine(x, -tickLength, x, tickLength);
        
        // 只在靠近视图中心时绘制数字标签
        if (x != 0 && visibleRect.contains(QPointF(x, 0))) {
            painter->drawText(QRectF(x - 20, tickLength + 2, 40, 20),
                            Qt::AlignCenter, QString::number(x / Pointscale));
        }
    }

    // 绘制Y轴刻度
    for (int y = startY; y <= endY; y += Pointscale) {
        if (y == 0) continue;
        painter->drawLine(-tickLength, y, tickLength, y);
        
        // 只在靠近视图中心时绘制数字标签
        if (visibleRect.contains(QPointF(0, y))) {
            painter->drawText(QRectF(-tickLength - 50, y - 7, 40, 40),
                            Qt::AlignRight, QString::number(-y / Pointscale));
        }
    }

    // 原点标记
    if (visibleRect.contains(QPointF(0, 0))) {
        painter->drawText(QRectF(0, tickLength + 2, 20, 20), Qt::AlignCenter, "0");
    }

    // 绘制X标签（始终在视图右侧）
    qreal viewRight = halfWidth - offset.x()/scale;
    qreal labelX = viewRight - 20;
    qreal labelY = -20;
    painter->drawText(QRectF(labelX - 10, labelY - 10, 20, 20), 
                    Qt::AlignCenter, "X");

    // 绘制Y标签（始终在视图上方）
    qreal viewTop = -halfHeight - offset.y()/scale;
    qreal labelY2 = viewTop + 20;
    qreal labelX2 = 20;
    painter->drawText(QRectF(labelX2 - 10, labelY2 - 10, 20, 20), 
                    Qt::AlignCenter, "Y");

    painter->restore();
}

void CoordinateWidget::drawCross(QPainter *painter, const QPointF &point) {
    painter->save();

    // 设置画笔颜色为红色，宽度随缩放调整
    QPen pen(Qt::red);
    pen.setWidthF(2.0 / scale);  // 缩放时保持线宽合适
    painter->setPen(pen);

    int size = (int)(5 * ((double)Pointscale / 50));  // 叉的大小随网格缩放
    int scaledX = point.x() * Pointscale;
    int scaledY = -point.y() * Pointscale;

    painter->drawLine(scaledX - size, scaledY - size, scaledX + size, scaledY + size);
    painter->drawLine(scaledX - size, scaledY + size, scaledX + size, scaledY - size);

    painter->restore();
}



void CoordinateWidget::drawCircle(QPainter *painter, const QPointF &point, int label) {
    painter->save();

    painter->setPen(getColorForLabel(label));
    painter->setBrush(getColorForLabel(label));

    int scaledX = point.x() * Pointscale;
    int scaledY = -point.y() * Pointscale;

    // 绘制一个半径为3像素的椭圆（即小圆点）
    painter->drawEllipse(QPoint(scaledX, scaledY), (int)(3 * ((double)Pointscale / 50)), (int)(3 * ((double)Pointscale / 50)));
    painter->restore();
}

void CoordinateWidget::drawCore(QPainter *painter, const QPointF &point, int label) {
    painter->save();

    painter->setPen(getColorForLabel(label));

    int scaledX = point.x() * Pointscale;
    int scaledY = -point.y() * Pointscale;

    painter->drawEllipse(QPoint(scaledX, scaledY), (int)(eps * Pointscale), (int)(eps * Pointscale));
    painter->restore();
}

void CoordinateWidget::drawLegend(QPainter *painter) {
    painter->save();
    
    // 图例样式设置
    QFont legendFont = painter->font();
    legendFont.setBold(true);
    painter->setFont(legendFont);
    
    // 图例位置和尺寸
    const int legendWidth = 150;
    const int itemHeight = 25;
    const int margin = 15;
    
    QSet<int> unique;
    for (int label : labels) {
        unique.insert(label);
    }
    QList<int> uniqueLabels = unique.values();
    int legendHeight = margin * 2 + 30 + uniqueLabels.size() * itemHeight;
    QRect legendRect(width() - legendWidth - margin, margin, legendWidth, legendHeight);
    
    // 绘制阴影效果
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 50));
    painter->drawRoundedRect(legendRect.translated(3, 3), 5, 5);
    
    // 绘制背景
    painter->setPen(Qt::black);
    painter->setBrush(QColor(255, 255, 255, 230));
    painter->drawRoundedRect(legendRect, 5, 5);
    
    // 绘制标题
    painter->drawText(legendRect.adjusted(margin, margin, -margin, -margin), 
                    Qt::AlignHCenter | Qt::AlignTop, 
                    "Color Legend");
    
    // 绘制每个标签项
    int yPos = legendRect.top() + margin + 30;
    for (int label : uniqueLabels) {
        // 颜色方块
        QRect colorRect(legendRect.left() + margin, yPos, 18, 18);
        painter->setBrush(getColorForLabel(label));
        painter->drawRoundedRect(colorRect, 3, 3);
        
        // 标签文本
        QString text = (label == -1) ? "Unlabeled" : QString("Label %1").arg(label);
        painter->drawText(QRect(legendRect.left() + margin + 25, yPos, 
                          legendRect.width() - 2 * margin - 25, 18),
                         Qt::AlignLeft | Qt::AlignVCenter, text);
        
        yPos += itemHeight;
    }
    
    painter->restore();
}

void CoordinateWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.translate(width() / 2 + offset.x(), height() / 2 + offset.y());
    painter.scale(scale, scale);

    drawGrid(&painter);
    drawAxes(&painter);

    for(int i=0; i<points.size(); i++){
        drawCircle(&painter, points[i], labels[i]);
        if(drawAuxi && !point_features.empty() && point_features[i] == Corepoint){
            drawCore(&painter, points[i], labels[i]);
        }
    }

    if(drawAuxi){
        for (const auto& center : centers) {
            if (center.size() >= 2) {
                QPointF pt(center[0], center[1]);
                drawCross(&painter, pt);
            }
        }
    }

    painter.resetTransform();
    painter.setRenderHint(QPainter::Antialiasing);
    drawLegend(&painter);

}

void CoordinateWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        lastMousePosition = event->pos();
        dragging = true;
    }
}

void CoordinateWidget::mouseMoveEvent(QMouseEvent *event) {
    if (dragging) {
        QPointF delta = event->pos() - lastMousePosition;
        offset += delta;
        lastMousePosition = event->pos();
        update();
    }
}

void CoordinateWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragging = false;
    }
}

void CoordinateWidget::wheelEvent(QWheelEvent *event) {
    const double factor = 1.1;

    if (event->angleDelta().y() > 0) {
        scale *= factor;
    } else {
        scale /= factor;
    }

    scale = qBound(0.1, scale, 10.0);
    emit scaleChanged(scale);
    update();
}