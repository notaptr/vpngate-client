#include "mainwindow.h"

#include <QApplication>
#include <cstdio>

int main(int argc, char *argv[])
{
    QCoreApplication::setSetuidAllowed(true);
    QApplication a(argc, argv);

    printf("%s \n", BUILD);

    MainWindow w;
    w.show();
    return a.exec();
}
