#include "include/ClientData.h"
#include <QTcpSocket>
#include <QTextStream>
#include <QObject>
#include <QAbstractSocket>
#include <iostream>
#include <QCoreApplication>
#include <QTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>


ClientData::ClientData(const QString& host, int port) : m_NextBlockSize(0)
{
    m_tcpSocket = new QTcpSocket(this); // создание сокета
    m_tcpSocket->connectToHost(host, port); // соединяемся с сервером
    
    in = new QTextStream(stdin);
    out = new QTextStream(stdout);
    bufferResponse = new QByteArray;

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

        // ловим ответ от сервера в json и записываем в буфер

        /*
            Ответ 2 видов json:
            {
                "type" : "message",
                "response" : "..." <-- string обычный
            }

            {
                "type" : "information", <-- информация о таблице
                "columns" : ["...", "..."],
                "types" : {
                    "id" : "int",
                    "name" : "string",
                    ...
                }
            }

            После получения и обработки ответа, чистить буфер!!!
        */


        inData >> *bufferResponse;  // записали

        //*out << str << '\n' << Qt::flush;
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
                QByteArray command = createTable(); // сформировали запрос
                std::system("cls");
                //*out << "The request has been sent." << Qt::endl << Qt::flush;
                sendMessageToServer(command);       // отправили на сервер

                // ждем ответа от сервера json вида
                m_tcpSocket->waitForReadyRead();

                QJsonDocument response = QJsonDocument::fromJson(*bufferResponse);
                bufferResponse->clear();
                QJsonObject proc = response.object();

                if (proc.value("type").toString() == "message"){
                    *out << proc.value("response").toString() << Qt::flush;
                }
                *out << "\n\nPlease press any button...\n" << Qt::flush;
                std::cin.get();
                break;
            }

            case SHOW_TABLE:{
                QByteArray command = viewsTable();
                std::system("cls");
               // *out << "The request has been sent." << Qt::endl << Qt::flush;
                sendMessageToServer(command);       // отправили на сервер

                // ждем ответа от сервера
                m_tcpSocket->waitForReadyRead();
                QJsonDocument response = QJsonDocument::fromJson(*bufferResponse);
                bufferResponse->clear();
                QJsonObject proc = response.object();

                if (proc.value("type").toString() == "message"){
                    *out << proc.value("response").toString() << Qt::flush;
                }
                *out << "\n\nPlease press any button...\n" << Qt::flush;
                std::cin.get();
                break;
            }
            case VIEWS_STRUCT:{
                QByteArray command = viewsStruct();
                std::system("cls");
               // *out << "The request has been sent." << Qt::endl << Qt::flush;
                sendMessageToServer(command);       // отправили на сервер

                // ждем ответа от сервера
                m_tcpSocket->waitForReadyRead();
                QJsonDocument response = QJsonDocument::fromJson(*bufferResponse);
                bufferResponse->clear();
                QJsonObject proc = response.object();

                if (proc.value("type").toString() == "message"){
                    *out << proc.value("response").toString() << Qt::flush;
                }
                *out << "\n\nPlease press any button...\n" << Qt::flush;
                std::cin.get();
                break;
            }

            case INSERT_DATA:{
                QByteArray command = insertDataInTable();

                if (!command.isNull()){
                    std::system("cls");
                   // *out << "The request has been sent." << Qt::endl << Qt::flush;
                    sendMessageToServer(command);       // отправили на сервер

                    // ждем ответа от сервера
                    m_tcpSocket->waitForReadyRead();
                    QJsonDocument response = QJsonDocument::fromJson(*bufferResponse);
                    bufferResponse->clear();
                    QJsonObject proc = response.object();

                    if (proc.value("type").toString() == "message"){
                        *out << proc.value("response").toString() << Qt::flush;
                    }
                    *out << "\n\nPlease press any button...\n" << Qt::flush;
                    std::cin.get();
                } else if (command.isNull()){
                    *out << "\n\nPlease press any button...\n" << Qt::flush;
                    std::cin.get();
                }

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

void ClientData::sendMessageToServer(QByteArray& command){  // отправка сформированной команды
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
    *out << "3. Viewing the table structure (column names, types)\n";
    *out << "4. Inserting data into a table (insert into ... values ... )\n";
    *out << "5. Reading data from a table (select ... from ... where ... )\n";
    *out << "6. Changing the data in the table (update ... set ... where ...)\n";
    *out << "7. Deleting tables/rows/columns (delete ... / delete from ... where ... )\n\n";
    *out << "For reference, enter the command 'help'.\n";
    *out << "To exit, enter the command 'exit'.\n";
    *out << "command > ";
    out->flush();


}
