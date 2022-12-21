#include "include/ServerData.h"
#include "include/Server.h"


#include <QDir>
#include <QTextStream>
#include <QTcpSocket>
#include <QTcpServer>

Server::Server(QMap<QString, QReadWriteLock*>* locks, QMap<QString, QJsonObject*>* cache,QObject *parent) :
    QTcpServer(parent)
{

    conOutput = new QTextStream(stdout);
    //locks = new QMap<QString, QReadWriteLock*>();
    _locks = locks;
    _cache = cache;
}

void Server::startServer()
{
    int port = 2323;

    if(!this->listen(QHostAddress::Any, port)){
        *conOutput << "Listen error!\n" << Qt::flush;
        this->close();
        return;
    }
    *conOutput << "Server SaveData. (Listening on 0.0.0.0 " << port << ")\n" << Qt::flush;

    connect(this, SIGNAL(newConnection()), this, SLOT(slotConnection()));
    
}


void Server::slotConnection()
{
    QTcpSocket* client = this->nextPendingConnection();

    qintptr socketDescriptor = client->socketDescriptor(); // получаем дескриптор сокета
    

    ServerData *thread = new ServerData(socketDescriptor, _locks, _cache, this);
    
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    
    thread->start();
}
