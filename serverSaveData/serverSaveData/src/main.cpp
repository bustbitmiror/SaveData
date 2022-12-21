#include <QCoreApplication>
#include "include/ServerData.h"
#include "include/Server.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QMap<QString, QReadWriteLock*> locks;
    QMap<QString, QJsonObject*> cache;

    Server server(&locks, &cache);
    server.startServer();

    //delete locks;
    return a.exec();
}
