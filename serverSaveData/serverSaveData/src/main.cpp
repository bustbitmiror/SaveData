#include <QCoreApplication>
#include "include/ServerData.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ServerData server(2323);

    return a.exec();
}
