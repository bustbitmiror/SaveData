#include "include/ServerData.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QObject>
#include <QTime>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFile>


ServerData::ServerData(int port) : m_NextBlockSize(0)
{
    m_tcpServer = new QTcpServer(this);
    conOutput = new QTextStream(stdout);
    if(!m_tcpServer->listen(QHostAddress::Any, port)){

        *conOutput << "Listen error!\n" << Qt::flush;
        m_tcpServer->close();
        return;
    }
    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
    *conOutput << "Server SaveData. (Listening on 0.0.0.0 " << port << ")\n" << Qt::flush;

    // Проверка папки tables и создание её
    dir = new QDir();
    dir->current();

    if(!dir->cd("tables")){
        dir->mkdir("tables");
        dir->cd("tables");
    } else {
        dir->cd("tables");
    }

}

// Обрабатывает каждое подключение 
void ServerData::slotNewConnection(){
    QTcpSocket* pClientSocket = m_tcpServer->nextPendingConnection();

    connect(pClientSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    connect(pClientSocket, SIGNAL(readyRead()), this, SLOT(slotReadClient()));

    *conOutput << QTime::currentTime().toString() << " New Connection! Client: " << pClientSocket->localAddress().toString() << '\n' << Qt::flush;

    QJsonObject obj;
    obj.insert("type", "CREATE");
    obj.insert("name", "people");
    QJsonArray columns = {"id", "firstname", "lastname"};
    QJsonArray typesColumns = {"int", "string", "string"};
    obj.insert("columns", columns);
    obj.insert("typesColumns", typesColumns);


    QJsonDocument jsonRequest(obj);

    QJsonObject proc = jsonRequest.object();
    QJsonValue type = proc.value("type");

    if(proc.value("type").toString() == "CREATE"){
        *conOutput << proc.value("type").toString() << Qt::endl << Qt::flush;

        QStringList listFiles = dir->entryList(QDir::Files);
        QString tableName = proc.value("name").toString() + ".sddb";

        foreach (QString file, listFiles){
            *conOutput << QFile::exists(file) << " " << file << Qt::endl << Qt::flush;

            // проверка на существование таблицы
            if (file == tableName){
                *conOutput << file << " exist"<< Qt::endl << Qt::flush;
                // Откидываем сообщение о существовании таблицы и что пользователь дурак
                // и завершаем действие
                break;
            }
        }

        // формируем json и создаем файл

        QJsonObject object;
        QJsonObject jsonTypes;
        QJsonArray columns = proc.value("columns").toArray();
        QJsonArray typesColumns = proc.value("typesColumns").toArray();

        for(int i = 0; i < typesColumns.size(); i++){
            jsonTypes.insert(columns.at(i).toString(), typesColumns.at(i).toString());
        }

        object.insert("types", jsonTypes);
        object.insert("data", QJsonArray());

        QJsonDocument table(object);
        *conOutput << dir->currentPath() + "/tables/" + tableName << Qt::endl << Qt::flush;
        QFile file(dir->currentPath() + "/tables/" + tableName);

        if(!file.open(QIODevice::ReadWrite)){
            *conOutput << "File open error!" << Qt::endl << Qt::flush;
        }

        file.resize(0);
        file.write(table.toJson());
        file.close();

    }
}

// Обрабатывает ввод данных от каждого пользователя
void ServerData::slotReadClient(){
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);

    for(;;){
        if(!m_NextBlockSize){
            if(pClientSocket->bytesAvailable() < sizeof(quint16)){
                break;

            }
            
            in >> m_NextBlockSize;
        }

        if(pClientSocket->bytesAvailable() < m_NextBlockSize){
            break;
        }

        QString str;

        in >> str;

        QString strMessage = QTime::currentTime().toString() + " " + str + '\n';
        *conOutput << "Client (" << pClientSocket->localAddress().toString() << "): " << strMessage << '\n' << Qt::flush;
        m_NextBlockSize = 0;
        
        //sendToClient(pClientSocket, "Server echo: " + str + "\n");
    }
}


void ServerData::sendToClient(QTcpSocket* pSocket, const QString& message){
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);

    out << quint16(0) << message;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    pSocket->write(arrBlock);
}

void ServerData::slotDisconnected(){
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();

    *conOutput << QTime::currentTime().toString() << " Client (" << pClientSocket->localAddress().toString() << ") disconnected.\n" << Qt::flush;

    pClientSocket->deleteLater();

}
