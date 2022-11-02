#include "include/ServerData.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QObject>
#include <QTime>
#include <QTextStream>


ServerData::ServerData(int port) : m_NextBlockSize(0)
{
    m_tcpServer = new QTcpServer(this);
    conOutput = new QTextStream(stdout);
    if(!m_tcpServer->listen(QHostAddress::Any, port)){

        *conOutput << "Listen error!\n";
        conOutput->flush();
        m_tcpServer->close();
        return;
    }
    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
    *conOutput << "Server SaveData. (Listening on 0.0.0.0 " << port << ")\n";
    conOutput->flush();
}

// Обрабатывает каждое подключение 
void ServerData::slotNewConnection(){
    QTcpSocket* pClientSocket = m_tcpServer->nextPendingConnection();

    connect(pClientSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    connect(pClientSocket, SIGNAL(readyRead()), this, SLOT(slotReadClient()));

    *conOutput << QTime::currentTime().toString() << " New Connection! Client: " << pClientSocket->localAddress().toString() << '\n';
    conOutput->flush();
}

// Обрабатывает ввод данных от каждого пользователя
void ServerData::slotReadClient(){
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);

    for(;;){
        if(!m_NextBlockSize){
            if(pClientSocket->bytesAvailable() < sizeof(quint16)){
                break;

            }
            
            in >> m_NextBlockSize;
        }

        if(pClientSocket->bytesAvailable() < m_NextBlockSize){
            break;
        }

        QString str;

        in >> str;

        QString strMessage = QTime::currentTime().toString() + " " + str + '\n';
        *conOutput << "Client (" << pClientSocket->localAddress().toString() << "): " << strMessage << '\n';
        conOutput->flush();
        m_NextBlockSize = 0;
        
        //sendToClient(pClientSocket, "Server echo: " + str + "\n");
    }
}


void ServerData::sendToClient(QTcpSocket* pSocket, const QString& message){
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);

    out << quint16(0) << message;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    pSocket->write(arrBlock);
}

void ServerData::slotDisconnected(){
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();

    *conOutput << QTime::currentTime().toString() << " Client (" << pClientSocket->localAddress().toString() << ") disconnected.\n";
    conOutput->flush();

    pClientSocket->deleteLater();

}
