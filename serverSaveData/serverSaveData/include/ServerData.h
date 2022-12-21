#pragma once

#include <QObject>
#include <QThread>
#include <QMap>

class QTcpSocket;
class QTcpServer;
class QTextStream;
class QDataStream;
class QDir;
class QReadWriteLock;

class ServerData : public QThread {
Q_OBJECT

private:
    QTcpSocket* socket;
    qintptr socketDescriptor;

    quint16 m_NextBlockSize;
    QTextStream* conOutput;
    QDir* dir;
    QMap<QString, QReadWriteLock*>* locks;
    QMap<QString, QJsonObject*>* cache;

private:
    void sendToClient(QTcpSocket* pSocket, QByteArray& response);
    QByteArray jsonResponse(QString message);


public:
    explicit ServerData(qintptr ID, QMap<QString, QReadWriteLock*>* locks, QMap<QString, QJsonObject*>* cache, QObject *parent = 0);
    void run();


private slots:
    //void slotNewConnection();
    void slotReadClient();
    void slotDisconnected();
};
