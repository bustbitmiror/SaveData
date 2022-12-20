#pragma once

#include <QObject>
#include <QThread>
#include <QMap>

class QTcpSocket;
class QTcpServer;
class QTextStream;
class QDataStream;
class QDir;
//class QMap;
class QReadWriteLock;
//class QFile;

class ServerData : public QThread {
Q_OBJECT

private:
    QTcpSocket* socket;
    qintptr socketDescriptor;

    quint16 m_NextBlockSize;
    QTextStream* conOutput;
    QDir* dir;
    QMap<QString, QReadWriteLock*>* locks;
    //QFile* logFile;

private:
    void sendToClient(QTcpSocket* pSocket, QByteArray& response);
    QByteArray jsonResponse(QString message);
    //QByteArray jsonResponse(QString type, QByteArray& information);


public:
    explicit ServerData(qintptr ID, QMap<QString, QReadWriteLock*>* locks, QObject *parent = 0);
    void run();
    //virtual ~ServerData(){}


private slots:
    //void slotNewConnection();
    void slotReadClient();
    void slotDisconnected();
};
