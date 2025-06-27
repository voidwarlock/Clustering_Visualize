#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>         // 主窗口基类
#include <QPushButton>          // 按钮控件
#include "coordinatetwidget.h"  // 自定义坐标绘图区域（显示点和聚类结果）
#include <QToolButton>          // 带菜单或下拉选项的按钮
#include <QMenu>                // 菜单控件
#include <QAction>              // 菜单项动作
#include <QMessageBox>          // 显示提示信息框
#include <QLineEdit>            // 输入框控件
#include "clustering/Cluster.h" // 聚类算法相关头文件（包含 ClusteringParams 等结构体）
#include <QHBoxLayout>          // 水平布局管理器
#include <QVBoxLayout>          // 垂直布局管理器
#include <QCheckBox>            // 复选框控件
#include <QLabel>               // 标签控件
#include <QSlider>              // 滑动条控件
#include <QFont>                // 字体设置
#include <atomic>               // 原子类型支持（用于多线程控制）

/**
 * @class MainWindow
 * @brief 应用程序主窗口类，负责界面布局与用户交互逻辑。
 *
 * 该类继承自 QMainWindow，作为整个聚类可视化工具的核心界面，
 * 包含绘图区域、参数输入、按钮操作、菜单栏等功能模块。
 */
class MainWindow : public QMainWindow {
    Q_OBJECT  // 启用 Qt 的信号与槽机制

public:
    /**
     * 构造函数：初始化主窗口及其所有 UI 组件
     * @param parent 父级 QWidget，默认为 nullptr
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * 析构函数：释放资源
     */
    ~MainWindow();

private slots:
    /**
     * 清除绘图区域的所有点和聚类结果
     */
    void onClearClicked();

    /**
     * 接收来自 CoordinateWidget 的点变化信号，更新本地存储的点数据
     * @param points 新的点集列表
     */
    void handlePointsChanged(const QList<QPointF> &points);

    /**
     * 加载预设数据点文件（如 CSV 或 TXT）
     */
    void onLoadButton_clicked();

    /**
     * 缩放按钮点击事件处理
     */
    void onscaleButton_clicked();

    /**
     * 应用当前输入的聚类参数并开始聚类
     */
    void applyButtonClicked();

    /**
     * 辅助按钮点击事件（具体用途取决于上下文）
     */
    void auxButtonClicked();

    /**
     * 显示按钮点击事件（可能用于切换视图或显示模式）
     */
    void displayButtonClicked();

    /**
     * 处理选择聚类算法菜单项的点击事件
     * @param action 被点击的 QAction 对象
     */
    void handleClusterLoad(QAction *action);

    /**
     * 处理选择归一化方法菜单项的点击事件
     * @param action 被点击的 QAction 对象
     */
    void handleNormLoad(QAction *action);

private:
    // ========== UI 控件声明 ==========

    CoordinateWidget *coordinateWidget; ///< 自定义绘图组件，用于显示点集和聚类结果
    QPushButton *clearButton;           ///< 清除按钮
    QToolButton *clusterButton;         ///< 聚类算法选择按钮（带下拉菜单）
    QPushButton *loadButton;            ///< 加载数据文件按钮
    QLineEdit *numberscale;             ///< 缩放比例输入框
    QPushButton *scaleButton;           ///< 执行缩放按钮
    QPushButton *applyButton;           ///< 应用聚类参数按钮
    QPushButton *auxButton;             ///< 辅助功能按钮
    QPushButton *displayButton;         ///< 显示控制按钮
    QLineEdit *filePathLineEdit;        ///< 显示当前加载文件路径的文本框
    QLineEdit *selectedAlgorithmLineEdit; ///< 显示当前选择聚类算法的文本框
    QMenu *menu;                        ///< 聚类算法菜单
    QSlider* speedSlider;              ///< 控制动画速度的滑动条
    QLabel *waitingLabel;               ///< 显示“等待中”等状态信息的标签
    QFont buttonFont;                   ///< 所有按钮使用的字体样式
    QFont lineEditFont;                 ///< QLineEdit 使用的字体样式（可选）

    // ========== 数据与状态 ==========

    QList<QPointF> localPoints;         ///< 存储当前界面上的点集合
    ClusteringParams param;             ///< 当前聚类参数配置
    ClusterType clustertype;            ///< 当前选择的聚类算法类型
    ALLCluster* onecluster;             ///< 实际运行的聚类对象指针
    double oldscale;                    ///< 上一次的缩放值

    // ========== 布局 ==========

    QVBoxLayout *buttonLayout;          ///< 按钮组的垂直布局

    // ========== 参数输入控件 ==========

    QLineEdit* kValueLineEdit;          ///< KMeans 中的 K 值输入框
    QLineEdit* epsValueLineEdit;        ///< DBSCAN 中的 eps 值输入框
    QLineEdit* minptsValueLineEdit;     ///< DBSCAN 中的 minPts 值输入框
    QLineEdit* nClustersValueLineEdit;  ///< 层次聚类中的目标聚类数输入框
    QLineEdit* alphaValueLineEdit;      ///< MeanShift 中的 alpha 值输入框
    QLineEdit* dampingValueLineEdit;    ///< Affinity Propagation 中的 damping 值输入框
    QLineEdit* preferenceValueLineEdit; ///< Affinity Propagation 中的 preference 值输入框
    QLineEdit* tolValueLineEdit;        ///< 聚类收敛容忍度输入框
    QLineEdit* MaxiterValueLineEdit;    ///< 最大迭代次数输入框
    QLineEdit* normLineEdit;            ///< 显示当前选择的归一化方法
    QToolButton *normButton;            ///< 归一化方法选择按钮（带菜单）
    QMenu* normMenu;                    ///< 归一化方法菜单
    QCheckBox* initcheckBox;            ///< 是否使用初始中心的复选框
    QLineEdit* knnparamLineEdit;        ///< KNN 参数输入框
    QLineEdit* sigmaValueLineEdit;      ///< 高斯核参数 sigma 输入框
    QVBoxLayout* parameterLayout;       ///< 参数输入控件的垂直布局

    // ========== 状态标志 ==========

    bool isRunning;                     ///< 表示是否正在执行聚类任务
    std::atomic<bool> shouldStopAnimation; ///< 是否应停止动画
    std::atomic<bool> shouldStopClustering;///< 是否应中断聚类计算
    std::atomic<bool> isWindowClosing;     ///< 是否正在关闭窗口（用于线程安全退出）
    std::atomic<int> animationDelay;       ///< 动画帧间隔时间（毫秒）

};

#endif // MAINWINDOW_H