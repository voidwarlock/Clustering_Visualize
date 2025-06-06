#include <QApplication>
#include "mainwindow.h"
#include <QMetaType>

Q_DECLARE_METATYPE(std::vector<int>)
Q_DECLARE_METATYPE(std::vector<std::vector<double>>)
Q_DECLARE_METATYPE(std::vector<Pointtype>)
Q_DECLARE_METATYPE(std::vector<double>)
Q_DECLARE_METATYPE(std::vector<ClusterNode*>)

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    qRegisterMetaType<std::vector<int>>("std::vector<int>");
    qRegisterMetaType<std::vector<std::vector<double>>>("std::vector<std::vector<double>>");
    qRegisterMetaType<std::vector<Pointtype>>("std::vector<Pointtype>");
    qRegisterMetaType<std::vector<double>>("std::vector<double>");
    qRegisterMetaType<std::vector<ClusterNode*>>("std::vector<ClusterNode*>");
    
    MainWindow window;
    window.show();

    return app.exec();
}