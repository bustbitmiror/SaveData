#include <QCoreApplication>
#include "include/ClientData.h"
//#include <QTextStream>
#include <iostream>
#include <QThread>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    ClientData client("localhost", 2323); // подключились...






    return a.exec();
}
