#include <QCoreApplication>
#include "include/ServerData.h"
#include "include/Server.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server server;
    server.startServer();

    return a.exec();
}
