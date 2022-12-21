#ifndef MYSERVER_H
#define MYSERVER_H

#include <QTcpServer>
#include <QMap>
#include <QReadWriteLock>

class QTextStream;
class QDir;


class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QMap<QString, QReadWriteLock*>* locks, QMap<QString, QJsonObject*>* cache,QObject *parent = 0);
    void startServer();

public:
    QMap<QString, QReadWriteLock*>* _locks;
    QMap<QString, QJsonObject*>* _cache;

private:
    QTextStream* conOutput;

private slots:
    void slotConnection();
     
};

#endif // SERVER_H
