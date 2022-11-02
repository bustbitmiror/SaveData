#pragma once

#include <QObject>

class QTcpSocket;
class QTcpServer;
class QTextStream;
class QDataStream;

class ServerData : public QObject {
Q_OBJECT

private:
    QTcpServer* m_tcpServer;
    quint16 m_NextBlockSize;
    QTextStream* conOutput;

private:
    void sendToClient(QTcpSocket* pSocket, const QString& message);


public:
    ServerData(int port);
    //virtual ~ServerData(){}


private slots:
    void slotNewConnection();
    void slotReadClient();
    void slotDisconnected();
};
