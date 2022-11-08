#include "include/ClientData.h"
#include <QTcpSocket>
#include <QTextStream>
#include <QObject>
#include <QAbstractSocket>
#include <iostream>
#include <QCoreApplication>
#include <QTime>


ClientData::ClientData(const QString& host, int port) : m_NextBlockSize(0)
{
    m_tcpSocket = new QTcpSocket(this); // создание сокета
    m_tcpSocket->connectToHost(host, port); // соединяемся с сервером
    
    in = new QTextStream(stdin);
    out = new QTextStream(stdout);

    // соединяем слоты
    connect(m_tcpSocket, SIGNAL(connected()), SLOT(slotConnectedAndTransfer()));
    connect(m_tcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_tcpSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this , SLOT(slotError(QAbstractSocket::SocketError)));

}


void ClientData::slotReadyRead(){
    QDataStream inData(m_tcpSocket);

    for(;;){
        if (!m_NextBlockSize){
            if (m_tcpSocket->bytesAvailable() < sizeof(quint16)){
                break;
            }
            inData >> m_NextBlockSize;
        }

        if (m_tcpSocket->bytesAvailable() < m_NextBlockSize){
            break;
        }

        QString str;

        inData >> str;

        *out << str << '\n';
        out->flush();
        m_NextBlockSize = 0;
    }
}

void ClientData::slotError(QAbstractSocket::SocketError err){  // вероятно не работает exit!
    switch (err) {
    case QAbstractSocket::HostNotFoundError: {
        *out << "The host was not found.\n";
        out->flush();
        exit(1);
        break;
    }
    case QAbstractSocket::RemoteHostClosedError: {
        *out << "The remote host is closed.\n";
        out->flush();
        exit(2);
        break;
    }
    case QAbstractSocket::ConnectionRefusedError: {
        *out << "The connection was refused.\n";
        out->flush();
        exit(3);
        break;
    }
    default:
        *out << "Error: " << QString(m_tcpSocket->errorString()) << '\n';
        out->flush();
        exit(255);
        break;
    }
}



void ClientData::slotConnectedAndTransfer(){
    *out << "Welcome to SaveData!\n\n";
    out->flush();
    QThread::sleep(1);
    while(true){
//        *out << "in > ";
//        out->flush();
        Menu();

        QString str = in->readLine();
        in->flush();

        int choice;
        if (str.toLower() == "exit"){
            *out << "Exiting...\n";
            out->flush();
            exit(0);
        } else if (str.toLower() == "help"){

        } else {
           choice = str.toInt(); // toInt, далее switch(int)
        }

        switch (choice) {
            case CREATE_TABLE:{
                QString command = createTable();
                *out << command << '\n';
                out->flush();
                std::cin.get();
                break;
            }

            case SHOW_TABLE:{
                break;
            }

            case INSERT_DATA:{
                break;
            }

            case READ_DATA:{
                break;
            }

            case CHANGE_DATA:{
                break;
            }

            case DELETE_DATA:{
                break;
            }

            default:{
                break;
            }
        }

//        if(str.toLower() == "exit"){

//        } else {
//            sendMessageToServer(str);
//            m_tcpSocket->waitForReadyRead();
//        }

    }

}

void ClientData::sendMessageToServer(QString& command){
    QByteArray arrBlock;
    QDataStream dataOut(&arrBlock, QIODevice::WriteOnly);

    dataOut << quint16(0) << command;

    dataOut.device()->seek(0);
    dataOut << quint16(arrBlock.size() - sizeof(quint16));

    m_tcpSocket->write(arrBlock);

//    *out << "Send: " << str << '\n';
//    out->flush();
}

void ClientData::Menu(){
    /*

        1. Создание таблицы (create table)

            1) Ввод названия таблицы (также проверка существования)
            2) Ввод названий столбцов и типов (строчка столбцов, далее типы к каждому столбцу)

        2. Просмотр существующих таблиц
        3. Вставка данных в таблицу (insert into ... values ... )
        4. Чтение данных из таблицы (select ... from ... where ... )
        5. Изменение данных в таблице (update ... set ... where ...)
        6. Удаление таблиц/строк/столбов (delete ... / delete from ... where ... )

    */

    std::system("cls");
    *out << "1. Create table\n";
    *out << "2. Viewing existing tables\n";
    *out << "3. Inserting data into a table (insert into ... values ... )\n";
    *out << "4. Reading data from a table (select ... from ... where ... )\n";
    *out << "5. Changing the data in the table (update ... set ... where ...)\n";
    *out << "6. Deleting tables/rows/columns (delete ... / delete from ... where ... )\n\n";
    *out << "For reference, enter the command 'help'.\n";
    *out << "To exit, enter the command 'exit'.\n";
    *out << "command > ";
    out->flush();


}



//--------------- Commands----------------

QString ClientData::createTable(){
    std::system("cls");
    QString resultCommand = "create table ";

    QMap<QString, QString> columnsAndTypes;
    QString nameTable;
    QString namesColumns;

    *out << "Table name: ";
    out->flush();

    nameTable = in->readLine();
    in->flush();
    resultCommand += nameTable + " ";
    resultCommand += "(";

    *out << "Column names (separated by a space): ";
    out->flush();
    namesColumns = in->readLine();
    in->flush();
    QStringList column = namesColumns.split(u' ', Qt::SkipEmptyParts);

    QStringList typesColumns;

    while(typesColumns.size() != column.size()){
        *out << "Column type (separated by a space): ";
        out->flush();

        typesColumns = (in->readLine()).split(u' ', Qt::SkipEmptyParts);
        in->flush();

        if(typesColumns.size() != column.size()){
            *out << "Unequal number of columns and types!\n";
            out->flush();
        }
    }


    for (int i = 0; i < column.size(); i++){
        // нужна проверка на соответствие с типом доступным
        columnsAndTypes[column.at(i)] = typesColumns.at(i);


        resultCommand += column.at(i) + " " + typesColumns.at(i);

    }

    QMap<QString, QString>::const_iterator i = columnsAndTypes.constBegin();
    while (i != columnsAndTypes.constEnd()) {
        *out << i.key() << ": " << i.value() << Qt::endl << Qt::flush;
        //out->flush();
        ++i;
    }

    resultCommand += ");";

//    *out << nameTable << '\n';
//    out->flush();



    //std::cin.get();

    return resultCommand;
}





