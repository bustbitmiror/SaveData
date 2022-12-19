#include "include/ServerData.h"
#include "include/Server.h"


#include <QDir>
#include <QTextStream>
#include <QTcpSocket>
#include <QTcpServer>

Server::Server(QObject *parent) :
    QTcpServer(parent)
{

    conOutput = new QTextStream(stdout);


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

// This function is called by QTcpServer when a new connection is available. 
void Server::slotConnection()
{
    QTcpSocket* client = this->nextPendingConnection();

    qintptr socketDescriptor = client->socketDescriptor();
    // We have a new connection
    
    // Every new connection will be run in a newly created thread
    ServerData *thread = new ServerData(socketDescriptor, this);
    
    // connect signal/slot
    // once a thread is not needed, it will be beleted later
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    
    thread->start();
}
