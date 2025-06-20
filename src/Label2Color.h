#ifndef LABEL2COLOR_H
#define LABEL2COLOR_H

#include <QList>

inline QColor getColorForLabel(int label) {
    static const QList<QColor> colorTable = {
        // 基本颜色
        QColor(255,0,0),     // 红色
        QColor(0,255,0),     // 绿色
        QColor(0,0,255),     // 蓝色
        // 扩展颜色
        QColor(220,220,70),   // 黄色
        QColor(255,0,255),   // 品红
        QColor(0,255,255),   // 青色
        QColor(255,128,0),   // 橙色
        QColor(128,0,255),   // 紫色
        QColor(0,255,128),   // 春绿色
        QColor(255,0,128),    // 玫瑰红
        QColor(128,255,0),    // 黄绿色
        QColor(0,128,255),    // 天蓝色
        QColor(255,128,128),  // 浅红色
        QColor(128,255,128),  // 浅绿色
        QColor(128,128,255),   // 浅蓝色
        QColor(192,192,192),   // 银色
        QColor(255,215,0),     // 金色
        QColor(139,69,19),    // 棕色
        QColor(75,0,130),      // 靛蓝色
        QColor(220,20,60),     // 深红色
        QColor(0,100,0),       // 深绿色
        QColor(0,0,139),       // 深蓝色
        // 新增颜色
        QColor(147,112,219),   // 中紫色
        QColor(255,192,203),   // 粉红色
        QColor(165,42,42),     // 棕红色
        QColor(240,230,140),   // 卡其色
        QColor(245,245,220),   // 米色
        QColor(34,225,128),     // 青色 (已存在)
        QColor(255,255,0),     // 亮黄色
        QColor(0,128,128),     // 深青色
    };
    
    if(label == -1){
        return Qt::gray;
    }
    else if(label == -2){
        return Qt::black;
    }
    else{
        return colorTable.at(abs(label) % colorTable.size());
    }
}


#endif