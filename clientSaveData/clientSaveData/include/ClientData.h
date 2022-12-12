#pragma once

#include <QObject>
#include <QAbstractSocket>
#include <QThread>
#include <QTextStream>
#include <QLocale>
#include <QVector>
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
    QVector<QString> typesCol = {"int", "double", "string", "bool"};  // типы наших колонок
    QByteArray* bufferResponse;  // буфер ответа от сервера

public:
    ClientData(const QString& host, int port);


//commands, типа обработчик команд, общая функция и сами команды
public:
    enum Command {CREATE_TABLE = 1, SHOW_TABLE = 2, VIEWS_STRUCT = 3, INSERT_DATA = 4, READ_DATA = 5, CHANGE_DATA = 6, DELETE_DATA = 7};
    enum SIZE_TYPE {INT_SIZE = 11, STRING_SIZE = 40, BOOL_SIZE = 5};
    QByteArray createTable();
    QByteArray viewsTable();
    QByteArray insertDataInTable();
    QByteArray viewsStruct();
    QByteArray readData();


private:
    void sendMessageToServer(QByteArray& str);
    void Menu();
    void Help();



private slots:
    void slotConnectedAndTransfer();
    void slotReadyRead();
    //void slotSendToServer();
    void slotError(QAbstractSocket::SocketError err);



};
