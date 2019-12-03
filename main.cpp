#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
    w.show();

    return a.exec();
}
