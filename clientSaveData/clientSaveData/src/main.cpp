#include <QCoreApplication>
#include "include/ClientData.h"
//#include <QTextStream>
#include <iostream>
#include <QThread>
#include <QString>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    ClientData client(QString(argv[1]), 2323); // подключились...

    return a.exec();
}
