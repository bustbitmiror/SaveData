#include <QCoreApplication>
#include "include/ServerData.h"
#include "include/Server.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QMap<QString, QReadWriteLock*> locks;

    Server server(&locks);
    server.startServer();

    //delete locks;
    return a.exec();
}
