#pragma once

#include <QObject>
#include <QAbstractSocket>
#include <QThread>
#include <QTextStream>
#include <QLocale>
class QAbstractSocket;
class QTcpSocket;
class QTextStream;
class QDataStream;




class ClientData : public QObject{
Q_OBJECT

private:
    QTcpSocket* m_tcpSocket;
    quint16 m_NextBlockSize;
    QTextStream* out;
    QTextStream* in;  // не надо скорее всего, здесь только ответы от сервера

public:
    ClientData(const QString& host, int port);


//commands, типа обработчик команд, общая функция и сами команды
public:
    enum Command {CREATE_TABLE = 1, SHOW_TABLE = 2, INSERT_DATA = 3, READ_DATA = 4, CHANGE_DATA = 5, DELETE_DATA = 6};
    QString createTable();


private:
    void sendMessageToServer(QString& str);
    void Menu();
    void Help();



private slots:
    void slotConnectedAndTransfer();
    void slotReadyRead();
    //void slotSendToServer();
    void slotError(QAbstractSocket::SocketError err);



};
