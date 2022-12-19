#pragma once

#include <QObject>

class QTcpSocket;
class QTcpServer;
class QTextStream;
class QDataStream;
class QDir;
//class QFile;

class ServerData : public QObject {
Q_OBJECT

private:
    QTcpServer* m_tcpServer;
    quint16 m_NextBlockSize;
    QTextStream* conOutput;
    QDir* dir;
    //QFile* logFile;

private:
    void sendToClient(QTcpSocket* pSocket, QByteArray& response);
    QByteArray jsonResponse(QString message);
    //QByteArray jsonResponse(QString type, QByteArray& information);


public:
    ServerData(int port);
    //virtual ~ServerData(){}


private slots:
    void slotNewConnection();
    void slotReadClient();
    void slotDisconnected();
};
