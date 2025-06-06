#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include "coordinatetwidget.h"
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QLineEdit>
#include "clustering/Cluster.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onClearClicked();
    void handlePointsChanged(const QList<QPointF> &points);
    void onLoadButton_clicked();    
    void onscaleButton_clicked();
    void applyButtonClicked();
    void auxButtonClicked();
    void displayButtonClicked();
    void handleClusterLoad(QAction *action);
    void handleNormLoad(QAction *action);
private:
    CoordinateWidget *coordinateWidget;
    QPushButton *clearButton;
    QToolButton *clusterButton;
    QPushButton *loadButton;
    QLineEdit *numberscale; // 新增：用于输入缩放比例
    QPushButton *scaleButton;
    QPushButton *applyButton;
    QPushButton *auxButton;
    QPushButton *displayButton;
    QLineEdit *filePathLineEdit;
    QLineEdit *selectedAlgorithmLineEdit;
    QMenu *menu;
    QSlider* speedSlider;
    QFont buttonFont;  // 所有按钮用的字体
    QFont lineEditFont; // 所有 QLineEdit 用的字体（可选

    QList<QPointF> localPoints;
    ClusteringParams param;
    ClusterType clustertype;
    ALLCluster* onecluster;
    double oldscale;

    QVBoxLayout *buttonLayout;

    QLineEdit* kValueLineEdit; // 新增K值输入框
    QLineEdit* epsValueLineEdit; 
    QLineEdit* minptsValueLineEdit; 
    QLineEdit* nClustersValueLineEdit; 
    QLineEdit* alphaValueLineEdit; 
    QLineEdit* dampingValueLineEdit; 
    QLineEdit* preferenceValueLineEdit;
    QLineEdit* tolValueLineEdit;
    QLineEdit* MaxiterValueLineEdit;
    QLineEdit* normLineEdit;
    QToolButton *normButton;
    QMenu* normMenu;
    QCheckBox* initcheckBox;
    QLineEdit* knnparamLineEdit;
    QLineEdit* sigmaValueLineEdit;
    QVBoxLayout* parameterLayout; // 存储参数控件的布局

    bool isRunning;
    std::atomic<bool> shouldStopAnimation;
    std::atomic<bool> isWindowClosing;
    std::atomic<int> animationDelay{500};
};

#endif // MAINWINDOW_H