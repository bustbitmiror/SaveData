#ifndef MYSERVER_H
#define MYSERVER_H

#include <QTcpServer>

class QTextStream;
class QDir;

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    void startServer();

private:
    QTextStream* conOutput;
    QDir* dir;
private slots:
    void slotConnection();
     
};

#endif // SERVER_H
