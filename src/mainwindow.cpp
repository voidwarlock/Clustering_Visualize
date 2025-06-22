#include "mainwindow.h"
#include <QRandomGenerator>
#include <QFileDialog>
#include "LoadData.h"
#include "convertion.h"
#include <QMetaObject>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrentRun>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), buttonFont("Arial", 10), lineEditFont("Arial", 12){

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    QWidget *controlPanel = new QWidget();
    controlPanel->setMinimumWidth(400);
    controlPanel->setMinimumHeight(1000);
    mainLayout->addWidget(controlPanel);

    coordinateWidget = new CoordinateWidget();
    coordinateWidget->setMinimumWidth(1000);
    coordinateWidget->setMinimumHeight(1000);
    mainLayout->addWidget(coordinateWidget, 1);

    buttonLayout = new QVBoxLayout(controlPanel);

    clearButton = new QPushButton("Clear", this);
    clearButton->setFixedSize(400, 50);
    clearButton->setFont(buttonFont);
    buttonLayout->addWidget(clearButton);


    filePathLineEdit = new QLineEdit(this); // 初始化 QLineEdit
    filePathLineEdit->setPlaceholderText("File path: ");
    filePathLineEdit->setReadOnly(true);
    filePathLineEdit->setFixedSize(300, 50);
    filePathLineEdit->setFont(lineEditFont);

    loadButton = new QPushButton("Load", this);
    loadButton->setFixedSize(100, 50);
    loadButton->setFont(buttonFont);

    numberscale = new QLineEdit(this);
    numberscale->setPlaceholderText("something like 10");
    numberscale->setText("1");
    numberscale->setFixedSize(300, 50);
    numberscale->setFont(lineEditFont);

    scaleButton = new QPushButton("Change Scale", this);
    scaleButton->setFixedSize(100, 50);
    scaleButton->setFont(buttonFont);

    QHBoxLayout *firstRowLayout = new QHBoxLayout();
    firstRowLayout->addWidget(filePathLineEdit);
    firstRowLayout->addWidget(loadButton);

    // 创建第二个水平布局，用于 numberscale 和 scaleButton
    QHBoxLayout *secondRowLayout = new QHBoxLayout();
    secondRowLayout->addWidget(numberscale);
    secondRowLayout->addWidget(scaleButton);
    
    buttonLayout->addLayout(firstRowLayout);
    buttonLayout->addLayout(secondRowLayout);

    selectedAlgorithmLineEdit = new QLineEdit(this); // 初始化新文本框
    selectedAlgorithmLineEdit->setPlaceholderText("Cluster Algorithm:");
    selectedAlgorithmLineEdit->setReadOnly(true); // 设置为只读
    selectedAlgorithmLineEdit->setFixedSize(300, 50);
    selectedAlgorithmLineEdit->setFont(lineEditFont);
/////
    clusterButton = new QToolButton(this);
    clusterButton->setText("Cluster");
    clusterButton->setPopupMode(QToolButton::MenuButtonPopup);
    clusterButton->setFixedSize(100, 50);
    clusterButton->setFont(buttonFont);
    //loadButton->setFixedSize(100, 30);

    menu = new QMenu(this);
    menu->addAction("K_Means");
    menu->addAction("DBSCAN");
    menu->addAction("Agglomerative");
    menu->addAction("DPMM");
    menu->addAction("Affinity_Propagation");
    menu->addAction("Spectral");

    clusterButton->setMenu(menu);

    QHBoxLayout *thirdRowLayout = new QHBoxLayout();
    thirdRowLayout->addWidget(selectedAlgorithmLineEdit);
    thirdRowLayout->addWidget(clusterButton);

    buttonLayout->addLayout(thirdRowLayout);

    // 创建等待提示标签
    waitingLabel = new QLabel("\n\n\n\n\n\n\nOperating cluster, please wait...", this);
    waitingLabel->setStyleSheet("color: red; font-size: 20px;");
    waitingLabel->hide();  // 默认隐藏

    // 将这个容器加入 buttonLayout
    buttonLayout->addWidget(waitingLabel);

    buttonLayout->addStretch();

    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(menu, &QMenu::triggered, this, &MainWindow::handleClusterLoad);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::onLoadButton_clicked);
    connect(scaleButton, &QPushButton::clicked, this, &MainWindow::onscaleButton_clicked);
    connect(coordinateWidget, &CoordinateWidget::pointsChanged, this, &MainWindow::handlePointsChanged);

    clustertype = None;
    isRunning = false;
    shouldStopAnimation = false;
    isWindowClosing = false;
}

MainWindow:: ~MainWindow(){
    delete onecluster;
    isWindowClosing = true; // 标记窗口正在关闭
    if (isRunning) {
        shouldStopAnimation = true; // 提前终止动画线程
        QThread::msleep(600); // 等待一段时间让线程退出（可选）
    }
}


void MainWindow::onClearClicked() {
    filePathLineEdit->clear();
    filePathLineEdit->setPlaceholderText("File path: ");
    selectedAlgorithmLineEdit->clear();
    selectedAlgorithmLineEdit->setPlaceholderText("Cluster Algorithm:");
    coordinateWidget->setPoints(QList<QPointF>());
    coordinateWidget->setCenters(std::vector<std::vector<double>>());
    coordinateWidget->setPoint_features(std::vector<Pointtype>(), 0.0);
    coordinateWidget->setLabels(std::vector<int>()); // 清空
    coordinateWidget->setFlags(false);
    coordinateWidget->update();
    shouldStopAnimation = true;
    if (onecluster!=nullptr){
        delete onecluster;
        onecluster = nullptr;
    }
    
    
    if (parameterLayout) {
        // 先移除布局中的所有控件
        QLayoutItem* item;
        while (item = parameterLayout->takeAt(0)) {
            if (item->widget()) {
                delete item->widget(); // 删除控件
            }
            delete item; // 删除布局项
        }
        
        // 从主布局中移除parameterLayout
        buttonLayout->removeItem(parameterLayout);
        
        // 删除parameterLayout本身
        delete parameterLayout;
        parameterLayout = nullptr;
    }

    kValueLineEdit = nullptr;
    epsValueLineEdit = nullptr;
    minptsValueLineEdit = nullptr;
    nClustersValueLineEdit = nullptr;
    alphaValueLineEdit = nullptr;
    dampingValueLineEdit = nullptr;
    preferenceValueLineEdit = nullptr;
    tolValueLineEdit = nullptr;
    MaxiterValueLineEdit = nullptr;
    normLineEdit = nullptr;
    normButton = nullptr;
    normMenu = nullptr;
    sigmaValueLineEdit = nullptr;
    initcheckBox = nullptr;
    knnparamLineEdit = nullptr;

    if (applyButton) {
        buttonLayout->removeWidget(applyButton);
        delete applyButton;
        applyButton = nullptr;
    }
    if (auxButton) {
        buttonLayout->removeWidget(auxButton);
        delete auxButton;
        auxButton = nullptr;
    }
    if (displayButton) {
        buttonLayout->removeWidget(displayButton);
        delete displayButton;
        displayButton = nullptr;
    }
    if (speedSlider){
        buttonLayout->removeWidget(speedSlider);
        delete speedSlider;
        speedSlider = nullptr;
    }

    clustertype = None;
    selectedAlgorithmLineEdit->setText("");
}

void MainWindow::handlePointsChanged(const QList<QPointF> &points) {
    localPoints = points; // 主窗口保存一份副本
    qDebug() << "The amounts of current spots: " << points.size();
}

void MainWindow::onLoadButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Data File"), "/mnt/c/Users/sword/Desktop/Visual_Clustering/datas/", tr("Data Files (*.data *.csv)"));
    if (!filename.isEmpty()) {
        filePathLineEdit->setText(filename); // 更新 QLineEdit 文本
        numberscale->setText("1");
        oldscale = 1.0;
        // 加载数据...
        try {
            DataLoader loader(filename.toStdString());
            auto data = loader.load();
            shouldStopAnimation = true;
            if (!data.empty()) {
                QList<QPointF> points = loader.convertToQPointFList(data);
                localPoints = points;
                coordinateWidget->setPoints(points);
                coordinateWidget->setCenters(std::vector<std::vector<double>>());
                coordinateWidget->setPoint_features(std::vector<Pointtype>(), 0.0);
                coordinateWidget->setLabels(std::vector<int>(points.size(), -1)); // 清空
                coordinateWidget->setFlags(false);
                coordinateWidget->update();
                if (onecluster!=nullptr){
                    delete onecluster;
                    onecluster = nullptr;
                }
                Eigen::MatrixXd datas = fromQPointListToEigen(points);
                onecluster = new ALLCluster(datas, param);
            } else {
                QMessageBox::warning(this, "Warning", "Failed to load data.");
            }
        } catch (...) {
            QMessageBox::critical(this, "Error", "An error occurred while loading the file.");
        }
    }
}

void MainWindow::onscaleButton_clicked() {
    shouldStopAnimation = true;
    // 获取用户输入的比例
    bool ok;
    double scaleFactor = numberscale->text().toDouble(&ok);

    if (!ok || scaleFactor <= 0) {
        QMessageBox::warning(this, "error", "Please enter a valid positive number as the scaling ratio.");
        return;
    }

    if (localPoints.isEmpty()) {
        QMessageBox::information(this, "Notice", "There are currently no data points to scale.");
        return;
    }

    // 创建缩放后的数据
    QList<QPointF> scaledPoints;
    for (const QPointF &pt : localPoints) {
        scaledPoints.append(QPointF(pt.x() / oldscale * scaleFactor, pt.y() / oldscale * scaleFactor));
    }
    oldscale = scaleFactor;
    localPoints = scaledPoints;

    coordinateWidget->setPoints(localPoints);
    coordinateWidget->update();
}

void MainWindow::handleClusterLoad(QAction *action) {
    if (action) {
        QString selectedAlgorithm = action->text();
        selectedAlgorithmLineEdit->setText(selectedAlgorithm); // 更新文本框内容
        qDebug() << "Selected clustering algorithm:" << selectedAlgorithm;

        // 如果之前存在，先删除旧控件
        delete kValueLineEdit; 
        delete epsValueLineEdit;
        delete minptsValueLineEdit;
        delete nClustersValueLineEdit; 
        delete alphaValueLineEdit; 
        delete dampingValueLineEdit; 
        delete preferenceValueLineEdit;
        delete tolValueLineEdit;
        delete MaxiterValueLineEdit;
        delete normLineEdit;
        delete normButton;
        delete normMenu;
        delete sigmaValueLineEdit;
        delete initcheckBox;
        delete knnparamLineEdit;
        kValueLineEdit = nullptr;
        epsValueLineEdit = nullptr;
        minptsValueLineEdit = nullptr;
        nClustersValueLineEdit = nullptr; 
        alphaValueLineEdit = nullptr; 
        dampingValueLineEdit = nullptr; 
        preferenceValueLineEdit = nullptr;
        tolValueLineEdit = nullptr;
        MaxiterValueLineEdit = nullptr;
        normLineEdit = nullptr;
        normButton = nullptr;
        normMenu = nullptr;
        sigmaValueLineEdit = nullptr;
        initcheckBox = nullptr;
        knnparamLineEdit = nullptr;

        // 根据选择的算法更新界面
        if(selectedAlgorithm == "K_Means"){
            clustertype = k_means;
            // 创建K值输入框
            kValueLineEdit = new QLineEdit(this);
            kValueLineEdit->setPlaceholderText("Enter K value");
            kValueLineEdit->setFixedSize(400, 50);
            kValueLineEdit->setFont(lineEditFont);
            MaxiterValueLineEdit = new QLineEdit(this);
            MaxiterValueLineEdit->setPlaceholderText("Enter maxiter value");
            MaxiterValueLineEdit->setFixedSize(400, 50);
            MaxiterValueLineEdit->setFont(lineEditFont);
            tolValueLineEdit = new QLineEdit(this);
            tolValueLineEdit->setPlaceholderText("Enter tol value");
            tolValueLineEdit->setFixedSize(400, 50);
            tolValueLineEdit->setFont(lineEditFont);
            // 添加到布局中

            delete parameterLayout;
            parameterLayout = new QVBoxLayout(); // 假设这是放置参数控件的布局

            parameterLayout->addWidget(kValueLineEdit);
            parameterLayout->addWidget(MaxiterValueLineEdit);
            parameterLayout->addWidget(tolValueLineEdit);
            buttonLayout->addLayout(parameterLayout); // 将布局添加到主界面
        }
        if(selectedAlgorithm == "DBSCAN"){
            clustertype = dbscan;
                        
            epsValueLineEdit = new QLineEdit(this);
            epsValueLineEdit->setPlaceholderText("Enter eps value");
            epsValueLineEdit->setFixedSize(400, 50);
            epsValueLineEdit->setFont(lineEditFont);
            minptsValueLineEdit = new QLineEdit(this);
            minptsValueLineEdit->setPlaceholderText("Enter minpts value");
            minptsValueLineEdit->setFixedSize(400, 50);
            minptsValueLineEdit->setFont(lineEditFont);
            // 添加到布局中
            delete parameterLayout;
            parameterLayout = new QVBoxLayout();

            parameterLayout->addWidget(epsValueLineEdit);
            parameterLayout->addWidget(minptsValueLineEdit);
            buttonLayout->addLayout(parameterLayout); // 将布局添加到主界面
        }
        if(selectedAlgorithm == "Agglomerative"){
            clustertype = agglomerative;

            nClustersValueLineEdit = new QLineEdit(this);
            nClustersValueLineEdit->setPlaceholderText("Enter nClusters value");
            nClustersValueLineEdit->setFixedSize(400, 50);
            nClustersValueLineEdit->setFont(lineEditFont);
            // 添加到布局中
            delete parameterLayout;
            parameterLayout = new QVBoxLayout();

            parameterLayout->addWidget(nClustersValueLineEdit);
            buttonLayout->addLayout(parameterLayout); // 将布局添加到主界面
        }
        if(selectedAlgorithm == "DPMM"){
            clustertype = dpmm;

            alphaValueLineEdit = new QLineEdit(this);
            alphaValueLineEdit->setPlaceholderText("Enter alpha value");
            alphaValueLineEdit->setFixedSize(400, 50);
            alphaValueLineEdit->setFont(lineEditFont);
            MaxiterValueLineEdit = new QLineEdit(this);
            MaxiterValueLineEdit->setPlaceholderText("Enter maxiter value");
            MaxiterValueLineEdit->setFixedSize(400, 50);
            MaxiterValueLineEdit->setFont(lineEditFont);
            initcheckBox = new QCheckBox("KNN Init", this);
            initcheckBox->setChecked(false);
            initcheckBox->setStyleSheet(
                "QCheckBox {"
                "    font-size: 16px;"
                "    padding: 10px;"
                "    min-width: 120px;"
                "    min-height: 30px;"
                "}"
            );
            knnparamLineEdit = new QLineEdit(this);
            knnparamLineEdit->setPlaceholderText("Enter KNN K value");
            knnparamLineEdit->setFixedSize(400, 50);
            knnparamLineEdit->setFont(lineEditFont);
            knnparamLineEdit->hide(); // 初始隐藏
            
            // 添加到布局中
            delete parameterLayout;
            parameterLayout = new QVBoxLayout();
            
            parameterLayout->addWidget(alphaValueLineEdit);
            parameterLayout->addWidget(MaxiterValueLineEdit);
            parameterLayout->addWidget(initcheckBox);
            parameterLayout->addWidget(knnparamLineEdit);
            buttonLayout->addLayout(parameterLayout); // 将布局添加到主界面

            // 在 connect 中切换显示状态
            connect(initcheckBox, &QCheckBox::checkStateChanged, this, [=](Qt::CheckState state){
                if (state == Qt::Checked) {
                    knnparamLineEdit->show();
                    param.initType = KnnInit;
                } else {
                    knnparamLineEdit->clear();
                    knnparamLineEdit->hide();
                    param.initType = SingleInit;
                }
            });
        }
        if(selectedAlgorithm == "Affinity_Propagation"){
            clustertype = affinity_propagation;

            dampingValueLineEdit = new QLineEdit(this);
            dampingValueLineEdit->setPlaceholderText("Enter damping value");
            dampingValueLineEdit->setFixedSize(400, 50);
            dampingValueLineEdit->setFont(lineEditFont);
            preferenceValueLineEdit = new QLineEdit(this);
            preferenceValueLineEdit->setPlaceholderText("Enter preference value");
            preferenceValueLineEdit->setFixedSize(400, 50);
            preferenceValueLineEdit->setFont(lineEditFont);
            tolValueLineEdit = new QLineEdit(this);
            tolValueLineEdit->setPlaceholderText("Enter tol value");
            tolValueLineEdit->setFixedSize(400, 50);
            tolValueLineEdit->setFont(lineEditFont);
            MaxiterValueLineEdit = new QLineEdit(this);
            MaxiterValueLineEdit->setPlaceholderText("Enter maxiter value");
            MaxiterValueLineEdit->setFixedSize(400, 50);
            MaxiterValueLineEdit->setFont(lineEditFont);
            // 添加到布局中
            delete parameterLayout;
            parameterLayout = new QVBoxLayout();
            
            parameterLayout->addWidget(dampingValueLineEdit);
            parameterLayout->addWidget(preferenceValueLineEdit);
            parameterLayout->addWidget(tolValueLineEdit);
            parameterLayout->addWidget(MaxiterValueLineEdit);
            buttonLayout->addLayout(parameterLayout); // 将布局添加到主界面
        }
        if(selectedAlgorithm == "Spectral"){
            clustertype = spectral;

            kValueLineEdit = new QLineEdit(this);
            kValueLineEdit->setPlaceholderText("Enter K value");
            kValueLineEdit->setFixedSize(400, 50);
            kValueLineEdit->setFont(lineEditFont);
            sigmaValueLineEdit = new QLineEdit(this);
            sigmaValueLineEdit->setPlaceholderText("Enter sigma value");
            sigmaValueLineEdit->setFixedSize(400, 50);
            sigmaValueLineEdit->setFont(lineEditFont);
            normLineEdit = new QLineEdit(this); // 初始化新文本框
            normLineEdit->setText("NoNorm");
            normLineEdit->setReadOnly(true); // 设置为只读
            normLineEdit->setFixedSize(400, 50);
            normLineEdit->setFont(lineEditFont);

            normButton = new QToolButton(this);
            normButton->setText("Norm");
            normButton->setPopupMode(QToolButton::MenuButtonPopup);
            normButton->setFixedSize(100, 50);
            normButton->setFont(buttonFont);
            normMenu = new QMenu(this);
            normMenu->addAction("NoNorm");
            normMenu->addAction("RW");
            normMenu->addAction("SYM");
            normButton->setMenu(normMenu);
            // 添加到布局中
            delete parameterLayout;
            parameterLayout = new QVBoxLayout();
            
            parameterLayout->addWidget(kValueLineEdit);
            parameterLayout->addWidget(sigmaValueLineEdit);
            parameterLayout->addWidget(normButton);
            parameterLayout->addWidget(normLineEdit);
            buttonLayout->addLayout(parameterLayout); // 将布局添加到主界面

            connect(normMenu, &QMenu::triggered, this, &MainWindow::handleNormLoad);
        }
        if(clustertype != None){
            delete applyButton;
            delete auxButton;
            delete displayButton;
            delete speedSlider;
            applyButton = new QPushButton("Apply", this);
            applyButton->setFixedSize(400, 50);
            applyButton->setFont(buttonFont);
            buttonLayout->addWidget(applyButton);
            connect(applyButton, &QPushButton::clicked, this, &MainWindow::applyButtonClicked);
            
            auxButton = new QPushButton("Draw " + selectedAlgorithm + " Auxiliary", this);
            auxButton->setFixedSize(400, 50);
            auxButton->setFont(buttonFont);
            buttonLayout->addWidget(auxButton);
            connect(auxButton, &QPushButton::clicked, this, &MainWindow::auxButtonClicked);           
            
            displayButton = new QPushButton("Display", this);
            displayButton->setFixedSize(400, 50);
            displayButton->setFont(buttonFont);
            buttonLayout->addWidget(displayButton);
            connect(displayButton, &QPushButton::clicked, this, &MainWindow::displayButtonClicked);  
            
            speedSlider = new QSlider(Qt::Horizontal, this);
            speedSlider->setMinimum(1);
            speedSlider->setMaximum(100);
            speedSlider->setValue(50); // 默认速度
            buttonLayout->addWidget(speedSlider);
            animationDelay.store(500);
            connect(speedSlider, &QSlider::valueChanged, this, [this](int value) {
                int delay = 1000 - (value * 9.5); // 线性映射
                animationDelay.store(delay);
            });
        }
    }
}

void MainWindow::handleNormLoad(QAction *action){
    QString selectedNorm = action->text();
    normLineEdit->setText(selectedNorm); // 更新文本框内容
}

void MainWindow::applyButtonClicked() {
    qDebug() << "=== Clustering Parameters ===";
    bool ok = true;
    bool right = false;
    param.clustertype = clustertype;
    coordinateWidget->setFlags(false);
    shouldStopAnimation = true;
    waitingLabel->show();
    QApplication::processEvents();

    // K_Means 参数
    if (kValueLineEdit && !kValueLineEdit->text().isEmpty()) {
        param.k = kValueLineEdit->text().toInt(&right);
        if(param.k<=0) right = false;
        if (right) qDebug() << "K Value:" << param.k;
        else qDebug() << "Invalid K value";
        ok = ok && right;
    }
    
    // DBSCAN 参数
    if (epsValueLineEdit && !epsValueLineEdit->text().isEmpty()) {
        param.eps = epsValueLineEdit->text().toDouble(&right);
        if(param.eps<=0) right = false;
        if (right) qDebug() << "EPS Value:" << param.eps;
        else qDebug() << "Invalid EPS value";
        ok = ok && right;
    }
    
    if (minptsValueLineEdit && !minptsValueLineEdit->text().isEmpty()) {
        param.minpts = minptsValueLineEdit->text().toInt(&right);
        if(param.minpts<=0) right = false;
        if (right) qDebug() << "MinPts Value:" << param.minpts;
        else qDebug() << "Invalid MinPts value";
        ok = ok && right;
    }
    
    // Agglomerative 参数
    if (nClustersValueLineEdit && !nClustersValueLineEdit->text().isEmpty()) {
        param.nClusters = nClustersValueLineEdit->text().toInt(&right);
        if(param.nClusters<=0 || param.nClusters>localPoints.size()) right = false;
        if (right) qDebug() << "nClusters Value:" << param.nClusters;
        else qDebug() << "Invalid nClusters value";
        ok = ok && right;
    }
    
    // DPMM 参数
    if (alphaValueLineEdit && !alphaValueLineEdit->text().isEmpty()) {
        param.alpha = alphaValueLineEdit->text().toDouble(&right);
        if(param.alpha<=0) right = false;
        if (right) qDebug() << "Alpha Value:" << param.alpha;
        else qDebug() << "Invalid Alpha value";
        ok = ok && right;
    }
    
    // Affinity Propagation 参数
    if (dampingValueLineEdit && !dampingValueLineEdit->text().isEmpty()) {
        param.damping = dampingValueLineEdit->text().toDouble(&right);
        if(param.damping<0 || param.damping>=1) right = false;
        if (right) qDebug() << "Damping Value:" << param.damping;
        else qDebug() << "Invalid Damping value";
        ok = ok && right;
    }
    
    if (preferenceValueLineEdit && !preferenceValueLineEdit->text().isEmpty()) {
        bool inside_right;
        param.preference = preferenceValueLineEdit->text().toDouble(&inside_right);
        if (inside_right) qDebug() << "Preference Value:" << param.preference;
        else {
            param.preference = MEDIAN;
            qDebug() << "Invalid Preference value, use MEDIAN";
        }
    }else{
        param.preference = MEDIAN;
    }

    // 公共参数 Tol & Maxiter
    if (tolValueLineEdit && !tolValueLineEdit->text().isEmpty()) {
        param.tol = tolValueLineEdit->text().toDouble(&right);
        if (right) qDebug() << "Tol Value:" << param.tol;
        else qDebug() << "Invalid Tol value";
        ok = ok && right;
    }
    
    if (MaxiterValueLineEdit && !MaxiterValueLineEdit->text().isEmpty()) {
        bool inside_right;
        param.maxiter = MaxiterValueLineEdit->text().toInt(&inside_right);
        if (inside_right) qDebug() << "Maxiter Value:" << param.maxiter;
        else {
            param.maxiter = 100;
            qDebug() << "Invalid Maxiter value";
        }
    } else{
        param.maxiter = 100;
    }

    // Spectral 参数
    if (sigmaValueLineEdit && !sigmaValueLineEdit->text().isEmpty()) {
        param.sigma = sigmaValueLineEdit->text().toDouble(&right);
        if(param.sigma <= 0) right = false;
        if (right) qDebug() << "Sigma Value:" << param.sigma;
        else qDebug() << "Invalid Sigma value";
        ok = ok && right;
    }
    
    // Norm 类型
    if (normLineEdit && !normLineEdit->text().isEmpty()) {
        QString text = normLineEdit->text();
        if (text == "NoNorm") param.normType = NoNorm;
        else if (text == "RW") param.normType = RW;
        else if (text == "SYM") param.normType = SYM;
        qDebug() << "Norm Type:" << text;
    }else{
        param.normType = NoNorm;
    }

    if (knnparamLineEdit && !knnparamLineEdit->text().isEmpty()){
        param.n_neighbors = knnparamLineEdit->text().toInt(&right);
        if(param.n_neighbors <= 0) right = false;
        if (right) qDebug() << "n_neighbors Value:" << param.n_neighbors;
        else qDebug() << "Invalid n_neighbors value";
        ok = ok && right;
    }
    if (param.initType == KnnInit && param.n_neighbors <= 0){
        ok = false;
    }

    if(ok && right){
        Eigen::MatrixXd datas = fromQPointListToEigen(localPoints);
        if(datas.rows()>0){
            onecluster->setDatas(datas);
            onecluster->setParams(param);
            onecluster->start();  
            coordinateWidget->setLabels(onecluster->labels);
        }
    }
    waitingLabel->hide();
    qDebug() << "============================";
}


void MainWindow::auxButtonClicked() {
    if (onecluster) {  // 先判断 onecluster 是否为空指针
        coordinateWidget->setFlags(true);
        coordinateWidget->setCenters(onecluster->centers);
        coordinateWidget->setPoint_features(onecluster->point_features, param.eps);
        coordinateWidget->setProbs(onecluster->probs);
        coordinateWidget->setRoots(onecluster->roots, param.nClusters);
    } else {
        // 可选：处理 onecluster 不存在的情况，比如提示用户加载数据
        qDebug() << "Error: onecluster is null. Please load cluster data first.";
    }
}


void MainWindow::displayButtonClicked() {
    if (!isRunning) {
        // 当前未运行 → 启动动画
        isRunning = true;
        shouldStopAnimation = false;

        QThreadPool::globalInstance()->start([this]() {
            if (onecluster){
                for (int i = 0; i < onecluster->label_history.size(); ++i) {
                    if (isWindowClosing.load()) break;
                    if (shouldStopAnimation.load()) break;
                    if (!coordinateWidget) continue;
                    

                    QMetaObject::invokeMethod(coordinateWidget, "setLabels", Qt::QueuedConnection,
                                            Q_ARG(std::vector<int>, onecluster->label_history[i]));

                    if (onecluster->center_history.size() == onecluster->label_history.size()) {
                        QMetaObject::invokeMethod(coordinateWidget, "setCenters", Qt::QueuedConnection,
                                                Q_ARG(std::vector<std::vector<double>>, onecluster->center_history[i]));
                    }
                    if (onecluster->point_feature_history.size() == onecluster->label_history.size()) {
                        QMetaObject::invokeMethod(coordinateWidget, "setPoint_features", Qt::QueuedConnection,
                                                Q_ARG(std::vector<Pointtype>, onecluster->point_feature_history[i]),
                                                Q_ARG(double, param.eps));
                    }
                    if (onecluster->prob_history.size() == onecluster->label_history.size()) {
                        QMetaObject::invokeMethod(coordinateWidget, "setProbs", Qt::QueuedConnection,
                                                Q_ARG(std::vector<double>, onecluster->prob_history[i]));
                    }
                    if (onecluster->root_history.size() == onecluster->label_history.size() && onecluster->num_history.size() == onecluster->label_history.size()) {
                        QMetaObject::invokeMethod(coordinateWidget, "setRoots", Qt::QueuedConnection,
                                                Q_ARG(std::vector<ClusterNode*>, onecluster->root_history[i]),
                                                Q_ARG(int, onecluster->num_history[i]));
                    }
                    QThread::msleep(animationDelay.load());
                    QCoreApplication::processEvents();
                }
            }
            else{
                qDebug() << "Error: onecluster is null. Please load cluster data first.";
            }
            // 回到主线程更新状态
            QMetaObject::invokeMethod(this, [this]() {
                isRunning = false; 
            }, Qt::QueuedConnection);
        });

    } else {
        // 当前正在运行 → 停止动画
        shouldStopAnimation = true;
    }
}