#include "include/ServerData.h"
#include "include/StreamTable.h"

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
#include <QFileInfo>
#include <QtAlgorithms>
#include <sstream>
#include <string>
#include <QDateTime>
#include <QThread>
//#include <csignal>
//#include <QCoreApplication>


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
    //std::signal(SIGINT, &sExit);
    //std::signal(SIGTERM, &sExit);
    *conOutput << "Server SaveData. (Listening on 0.0.0.0 " << port << ")\n" << Qt::flush;

    //logFile = new QFile;

    dir = new QDir();
    dir->current();

//    if(!QFile::exists(dir->absolutePath() + "/program.log")){
//        logFile->setFileName(dir->absolutePath() + "/program.log");
//        if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append)){
//            *conOutput << "Error open log file!\n" << Qt::flush;
//        }
//    } else {
//        logFile->setFileName(dir->absolutePath() + "/program.log");
//        if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append)){
//            *conOutput << "Error open log file!\n" << Qt::flush;
//        }
//    }

    // Проверка папки tables и создание её
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

//    // отдельный поток для каждого подключенного клиента
//    QThread* sockThread = new QThread(this);
//    pClientSocket->moveToThread(sockThread);
//    sockThread->start();

    connect(pClientSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    connect(pClientSocket, SIGNAL(readyRead()), this, SLOT(slotReadClient()));



    *conOutput << QTime::currentTime().toString() << " New Connection! Client: " << pClientSocket->peerAddress().toString() << " Thread: " << pClientSocket->thread() << '\n' << Qt::flush;

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

        // ловим запрос от клиента
        QByteArray request;
        in >> request;

        QJsonDocument jsonRequest = QJsonDocument::fromJson(request);

        QJsonObject proc = jsonRequest.object();
        QJsonValue type = proc.value("type");

        if(proc.value("type").toString() == "CREATE"){      // create database
            //*conOutput << proc.value("type").toString() << Qt::endl << Qt::flush;

            //QStringList listFiles = dir->entryList(QDir::Files);
            QString tableName = proc.value("name").toString();

            // проверка на существование таблицы
            if (!dir->exists(tableName)){
                dir->mkdir(tableName);

            } else {
                // Откидываем сообщение о существовании таблицы и что пользователь дурак
                // и завершаем действие

                QByteArray resp = jsonResponse("The " + tableName + " table exists!\n");
                sendToClient(pClientSocket, resp);
                m_NextBlockSize = 0;
                break;
            }


            // формируем json и создаем файл
            dir->cd(tableName);
            QJsonObject object;
            QJsonObject jsonTypes;
            QJsonArray columns = proc.value("columns").toArray();
            QJsonArray typesColumns = proc.value("typesColumns").toArray();

            for(int i = 0; i < typesColumns.size(); i++){
                jsonTypes.insert(columns.at(i).toString(), typesColumns.at(i).toString());
            }

            QJsonArray arrColumns;
            for (int i = 0; i < columns.size(); i++){
                arrColumns.append(columns.at(i));
            }

            object.insert("types", jsonTypes);
            object.insert("columns", arrColumns);

            QJsonDocument table(object);
            QFile file(dir->absolutePath() + "/types.entry");

            if(!file.open(QIODevice::ReadWrite)){
                *conOutput << "File open error!" << Qt::endl << Qt::flush;
            }

            file.resize(0);
            file.write(table.toJson());
            file.close();
            dir->cdUp();


            QByteArray resp = jsonResponse("The " + tableName + " table has been created.\n");
            sendToClient(pClientSocket, resp);
            *conOutput << QTime::currentTime().toString() << " Client (" << pClientSocket->peerAddress().toString() << ") : " << "The " + tableName + " table has been created.\n" << Qt::flush;

        } else if (proc.value("type").toString() == "INSERT") {         // insert
            //*conOutput << proc.value("type").toString() << Qt::endl << Qt::flush;


            //*conOutput << dir->absolutePath() << Qt::endl << Qt::flush;
            QString tableName = proc.value("name").toString();
            //*conOutput << QFile::exists(dir->absolutePath() + "/" + tableName) << Qt::endl << Qt::flush;
            if (!dir->exists(tableName)){
                //откидываем сообщении о несуществовании таблицы

                QByteArray resp = jsonResponse("The " + tableName + " table does not exist!");
                sendToClient(pClientSocket, resp);
                m_NextBlockSize = 0;
                break;
            }

            dir->cd(tableName);

            QFile fileTypes(dir->absolutePath() + "/types.entry");
            if(!fileTypes.open(QIODevice::ReadOnly | QIODevice::Text)){
                *conOutput << "File open error!" << Qt::endl << Qt::flush;
            }

            QString jsonTypes = fileTypes.readAll();
            fileTypes.close();

            QJsonDocument typesTable = QJsonDocument::fromJson(jsonTypes.toUtf8());
            QJsonObject mainObjTypes = typesTable.object();
            QJsonObject typesObj = mainObjTypes.value("types").toObject();
            QJsonArray columnsFileTypes = mainObjTypes.value("columns").toArray();


            QJsonArray valuesArr = proc.value("values").toArray();
            QJsonArray columnsArr = proc.value("columns").toArray();
            QJsonObject entry;

            // делаем запись columns - value
            for (int i = 0; i < columnsArr.size(); i++){
                if (columnsFileTypes.contains(columnsArr.at(i))){
                    //*conOutput << "col: " << columnsArr.at(i).toString() << " value: " << valuesArr.at(i).toString() << " type: " << valuesArr.at(i).type() << Qt::endl << Qt::flush;
                    if ((typesObj[columnsArr.at(i).toString()] == "int" || typesObj[columnsArr.at(i).toString()] == "double") /*&& (valuesArr.at(i).type() == QJsonValue::Type::Double)*/){
                        entry.insert(columnsArr.at(i).toString(), valuesArr.at(i).toDouble());
                    } else if ((typesObj[columnsArr.at(i).toString()] == "string") /*&& (valuesArr.at(i).type() == QJsonValue::Type::String)*/) {
                        entry.insert(columnsArr.at(i).toString(), valuesArr.at(i).toString());
                    } else if ((typesObj[columnsArr.at(i).toString()] == "bool") /*&& (valuesArr.at(i).type() == QJsonValue::Type::Bool)*/) {
                        entry.insert(columnsArr.at(i).toString(), valuesArr.at(i).toBool());
                    } else {
                        //че то не так
                        *conOutput << "error with types!" << Qt::endl << Qt::flush;
                    }

                }
            }


            for (int i = 0; i < columnsFileTypes.size(); i++){
                if (entry.value(columnsFileTypes.at(i).toString()) == QJsonValue::Undefined){
                    entry.insert(columnsFileTypes.at(i).toString(), QJsonValue::Null);
                }
            }


            QJsonDocument entryJson(entry);  // json запись

            //int countFile = dir->entryList(QDir::Files).size();
            //QString nameEntryFile = tableName + QString::number(countFile-1) + ".entry";
            QString nameEntryFile = tableName + "_" + QDateTime::currentDateTime().toString("ddMMyyyy_HHmmss") + ".entry";
            //*conOutput << nameEntryFile << Qt::endl << Qt::flush;
            QFile entryF(dir->absolutePath() + "/" + nameEntryFile);
            if(!entryF.open(QIODevice::WriteOnly)){
                *conOutput << "File open error!" << Qt::endl << Qt::flush;
            }

            entryF.write(entryJson.toJson());
            entryF.close();


            // нужно сообщение для ответа


            QByteArray resp = jsonResponse("The data is recorded.");
            sendToClient(pClientSocket, resp);
            *conOutput << QTime::currentTime().toString() << " Client (" << pClientSocket->peerAddress().toString() << ") : " << "Wrote the data to the table " + tableName + " .\n" << Qt::flush;
            dir->cdUp();




        } else if (proc.value("type").toString() == "VIEWS") {  // views database
            QString retMessage;
            QTextStream ret(&retMessage);
            QStringList listTable = dir->entryList(QDir::Dirs | QDir::NoDotAndDotDot);

            ret << "All tables:" << Qt::endl;
            for (int i = 0; i < listTable.size(); i++){
                ret << "\t" << listTable.at(i) << Qt::endl;
            }

            *conOutput << QTime::currentTime().toString() << " Client (" << pClientSocket->peerAddress().toString() << ") : " << "I looked at the existing tables.\n" << Qt::flush;
            QByteArray resp = jsonResponse(retMessage);
            sendToClient(pClientSocket, resp);

        } else if (proc.value("type").toString() == "SELECT"){   // select
            QString tableName = proc.value("name").toString();
            if (!dir->exists(tableName)){
                //откидываем сообщении о несуществовании таблицы
                QByteArray resp = jsonResponse("The " + tableName + " table does not exist!");
                sendToClient(pClientSocket, resp);
                m_NextBlockSize = 0;
                break;
            }

            QJsonArray col = proc.value("columns").toArray();

            dir->cd(tableName);

            QFile typesFile(dir->absolutePath() + "/types.entry");
            if(!typesFile.open(QIODevice::ReadOnly | QIODevice::Text)){
                *conOutput << "File not open!" << Qt::endl << Qt::flush;
            }
            QJsonDocument jsonTypes = QJsonDocument::fromJson(typesFile.readAll());
            typesFile.close();
            QJsonObject objJson = jsonTypes.object();
            QJsonArray columnsFromTypes = objJson.value("columns").toArray();
            QJsonObject objTypesFromTypes = objJson.value("types").toObject();


             //рисуем таблицу



            QStringList listEntry = dir->entryList(QDir::Files);
            QString outputTable;        // таблица для вывода, которую отправим пользователю
            //QTextStream out(&outputTable);
            //StreamTable st(out);
            std::ostringstream stream;
            StreamTable st(stream);

            if(col.size() == 1 && col.at(0).toString() == "*"){
                //st.SetCols(columnsFromTypes.size(), 15);
                for (int i = 0; i < columnsFromTypes.size(); i++){
                    if (objTypesFromTypes.value(columnsFromTypes.at(i).toString()) == "int" || objTypesFromTypes.value(columnsFromTypes.at(i).toString()) == "double"){

                        st.AddCol(11, true);
                        //*conOutput << columnsFromTypes.at(i).toString() << ": " << objJson.value(columnsFromTypes.at(i).toString()).toDouble() << " " << Qt::flush;
                    } else if (objTypesFromTypes.value(columnsFromTypes.at(i).toString()) == "bool") {
                        st.AddCol(5, true);
                        //*conOutput << columnsFromTypes.at(i).toString() << ": " << objJson.value(columnsFromTypes.at(i).toString()).toBool() << " " << Qt::flush;
                    } else if (objTypesFromTypes.value(columnsFromTypes.at(i).toString()) == "string") {
                        st.AddCol(40, true);
                        //*conOutput << columnsFromTypes.at(i).toString() << ": " << objJson.value(columnsFromTypes.at(i).toString()).toString() << " " << Qt::flush;
                    }

                }

            } else {
                for(int i = 0; i < col.size(); i++){

                    if(columnsFromTypes.contains(col.at(i))){

                        if (objTypesFromTypes.value(col.at(i).toString()) == "int" || objTypesFromTypes.value(col.at(i).toString()) == "double"){
                            st.AddCol(11, true);
                            //*conOutput << col.at(i).toString() << ": " << objJson.value(col.at(i).toString()).toDouble() << " " << Qt::flush;
                        } else if (objTypesFromTypes.value(col.at(i).toString()) == "bool") {
                            st.AddCol(5, true);
                            //*conOutput << col.at(i).toString() << ": " << objJson.value(col.at(i).toString()).toBool() << " " << Qt::flush;
                        } else if (objTypesFromTypes.value(col.at(i).toString()) == "string") {
                            st.AddCol(40, true);
                            //*conOutput << col.at(i).toString() << ": " << objJson.value(col.at(i).toString()).toString() << " " << Qt::flush;
                        }

                    }
                }
            }
            //st.SetCols(2, 15);
            st.MakeBorderExt(true);
            st.SetDelimRow(true, '-');
            st.SetDelimCol(true, '|');
            //st.SetDelimCol(false);
            //st.SetDelimRow(false);


            if (col.size() == 1 && col.at(0).toString() == "*"){
                for (int i = 0; i < columnsFromTypes.size(); i++){

                    st << columnsFromTypes[i].toString().toStdString();

                }
            } else {
                for(int i = 0; i < col.size(); i++){
                    if(columnsFromTypes.contains(col.at(i))){
                        st << col.at(i).toString().toStdString();
                    }
                }
            }





            foreach (QString entry, listEntry){
                if(entry == "types.entry"){
                    continue;
                }

                // тут нужна проверка на readLock
                QFile entryFile(dir->absolutePath() + "/" + entry);
                if(!entryFile.open(QIODevice::ReadOnly | QIODevice::Text)){
                    *conOutput << "File not open!" << Qt::endl << Qt::flush;
                }

                QJsonDocument jsonFile = QJsonDocument::fromJson(entryFile.readAll());
                entryFile.close();
                QJsonObject objJson = jsonFile.object();





                if (col.size() == 1 && col.at(0).toString() == "*"){

                    for (int i = 0; i < columnsFromTypes.size(); i++){
                        if (objTypesFromTypes.value(columnsFromTypes.at(i).toString()) == "int" || objTypesFromTypes.value(columnsFromTypes.at(i).toString()) == "double"){

                            st << objJson.value(columnsFromTypes.at(i).toString()).toDouble();
                            //*conOutput << columnsFromTypes.at(i).toString() << ": " << objJson.value(columnsFromTypes.at(i).toString()).toDouble() << " " << Qt::flush;
                        } else if (objTypesFromTypes.value(columnsFromTypes.at(i).toString()) == "bool") {
                            st << objJson.value(columnsFromTypes.at(i).toString()).toBool();
                            //*conOutput << columnsFromTypes.at(i).toString() << ": " << objJson.value(columnsFromTypes.at(i).toString()).toBool() << " " << Qt::flush;
                        } else if (objTypesFromTypes.value(columnsFromTypes.at(i).toString()) == "string") {
                            st << objJson.value(columnsFromTypes.at(i).toString()).toString().toStdString();
                            //*conOutput << columnsFromTypes.at(i).toString() << ": " << objJson.value(columnsFromTypes.at(i).toString()).toString() << " " << Qt::flush;
                        }

                    }

                } else {

                    for(int i = 0; i < col.size(); i++){

                        if(columnsFromTypes.contains(col.at(i))){

                            if (objTypesFromTypes.value(col.at(i).toString()) == "int" || objTypesFromTypes.value(col.at(i).toString()) == "double"){
                                st << objJson.value(col.at(i).toString()).toDouble();
                                //*conOutput << col.at(i).toString() << ": " << objJson.value(col.at(i).toString()).toDouble() << " " << Qt::flush;
                            } else if (objTypesFromTypes.value(col.at(i).toString()) == "bool") {
                                st << objJson.value(col.at(i).toString()).toBool();
                                //*conOutput << col.at(i).toString() << ": " << objJson.value(col.at(i).toString()).toBool() << " " << Qt::flush;
                            } else if (objTypesFromTypes.value(col.at(i).toString()) == "string") {
                                st << objJson.value(col.at(i).toString()).toString().toStdString();
                                //*conOutput << col.at(i).toString() << ": " << objJson.value(col.at(i).toString()).toString() << " " << Qt::flush;
                            }

                        }
                    }


                    //*conOutput << Qt::endl << Qt::flush;
                }

            }

            outputTable = QString::fromStdString(stream.str());
            dir->cdUp();
            QByteArray resp = jsonResponse(outputTable);
            sendToClient(pClientSocket, resp); // отправляем структуру
            *conOutput << QTime::currentTime().toString() << " Client (" << pClientSocket->peerAddress().toString() << ") : " << "I looked at the contents of the " + tableName + " table.\n" << Qt::flush;

            //*conOutput << outputTable << Qt::flush;


        } else if (proc.value("type").toString() == "VIEWSSTRUCT"){ // VIEWSSTRUCT

            QString tableName = proc.value("name").toString();

            if (!dir->exists(tableName)){
                //откидываем сообщении о несуществовании таблицы
                QByteArray responseMessage = jsonResponse("The " + tableName + " table does not exist!");
                sendToClient(pClientSocket, responseMessage);
                m_NextBlockSize = 0;
                break;
            }

            dir->cd(tableName);

            QFile fileTypes(dir->absolutePath() + "/types.entry");
            if(!fileTypes.open(QIODevice::ReadOnly | QIODevice::Text)){
                *conOutput << "File open error!" << Qt::endl << Qt::flush;
            }

            QString jsonTypes = fileTypes.readAll();
            fileTypes.close();

            QJsonDocument typesTable = QJsonDocument::fromJson(jsonTypes.toUtf8());
            QJsonObject mainObjTypes = typesTable.object();
            QJsonObject typesObj = mainObjTypes.value("types").toObject();
            QJsonArray columnsFileTypes = mainObjTypes.value("columns").toArray();

            QString retOutput;
            QTextStream ret(&retOutput);

            ret << "Columns: ";
            for (int i = 0; i < columnsFileTypes.size(); i++){
                ret << columnsFileTypes.at(i).toString() << " ";
            }
            ret << "\n\n";

            ret << "Types of columns:\n";
            for (int i = 0; i < columnsFileTypes.size(); i++){
                ret << "\t" << columnsFileTypes.at(i).toString() << " : " << typesObj.value(columnsFileTypes.at(i).toString()).toString() << Qt::endl;
            }


            QByteArray resp = jsonResponse(retOutput);
            sendToClient(pClientSocket, resp); // отправляем структуру
            *conOutput << QTime::currentTime().toString() << " Client (" << pClientSocket->peerAddress().toString() << ") : " << "I looked at the structure of the " + tableName + " table.\n" << Qt::flush;
            dir->cdUp();
        } else if (proc.value("type").toString() == "INFORMATION"){
            QString tableName = proc.value("name").toString();

            if (!dir->exists(tableName)){
                //откидываем сообщении о несуществовании таблицы
                QByteArray responseMessage = jsonResponse("The " + tableName + " table does not exist!");
                sendToClient(pClientSocket, responseMessage);
                m_NextBlockSize = 0;
                break;
            }


            dir->cd(tableName);

            QFile fileTypes(dir->absolutePath() + "/types.entry");
            if(!fileTypes.open(QIODevice::ReadOnly | QIODevice::Text)){
                *conOutput << "File open error!" << Qt::endl << Qt::flush;
            }

            QString jsonTypes = fileTypes.readAll();
            fileTypes.close();

            QJsonDocument typesTable = QJsonDocument::fromJson(jsonTypes.toUtf8());
            QJsonObject mainObjTypes = typesTable.object();
            mainObjTypes.insert("type", "information");
            typesTable.setObject(mainObjTypes);
            QByteArray responseInfo = typesTable.toJson();
            sendToClient(pClientSocket, responseInfo);

            dir->cdUp();
        } else if (proc.value("type").toString() == "DELETE"){
            QString tableName = proc.value("name").toString();

            if (!dir->exists(tableName)){
                //откидываем сообщении о несуществовании таблицы
                QByteArray responseMessage = jsonResponse("The " + tableName + " table does not exist!");
                sendToClient(pClientSocket, responseMessage);
                m_NextBlockSize = 0;
                break;
            }

            if(proc.value("deleteTable").toString() == "yes"){
                dir->cd(tableName);

                if(dir->removeRecursively()){
                    dir->cdUp();
                    *conOutput << QTime::currentTime().toString() << " Client (" << pClientSocket->peerAddress().toString() << ") : " << "Deleted the " + tableName + " table.\n" << Qt::flush;
                    QByteArray responseMessage = jsonResponse("Deleted the " + tableName + " table.");
                    sendToClient(pClientSocket, responseMessage);
                    m_NextBlockSize = 0;
                    break;
                } else {
                    dir->cdUp();
                    *conOutput << QTime::currentTime().toString() << " Client (" << pClientSocket->peerAddress().toString() << ") : " << "I tried to delete the " + tableName + " table. It didn't work out.\n" << Qt::flush;
                    QByteArray responseMessage = jsonResponse("I tried to delete the " + tableName + " table. It didn't work out.");
                    sendToClient(pClientSocket, responseMessage);
                    m_NextBlockSize = 0;
                    break;
                }

            }

            int countRemoveFile = 0;    // счетчик удаленных файлов
            QJsonObject objWhere = proc.value("where").toObject();
            QJsonArray col = proc.value("columns").toArray();
            dir->cd(tableName);

            QFile typesFile(dir->absolutePath() + "/types.entry");
            if(!typesFile.open(QIODevice::ReadOnly | QIODevice::Text)){
                // ошибка
                *conOutput << "File not open!" << Qt::endl << Qt::flush;
            }
            QJsonDocument jsonTypes = QJsonDocument::fromJson(typesFile.readAll());
            typesFile.close();
            QJsonObject objJson = jsonTypes.object();
            QJsonArray columnsFromTypes = objJson.value("columns").toArray();
            QJsonObject objTypesFromTypes = objJson.value("types").toObject();


            QStringList listEntry = dir->entryList(QDir::Files);
            foreach (QString entry, listEntry){
                if(entry == "types.entry"){
                    continue;
                }

                // тут нужна проверка на readLock
                QFile entryFile(dir->absolutePath() + "/" + entry);
                if(!entryFile.open(QIODevice::ReadOnly | QIODevice::Text)){
                    *conOutput << "File not open!" << Qt::endl << Qt::flush;
                }

                QJsonDocument jsonFile = QJsonDocument::fromJson(entryFile.readAll());
                entryFile.close();
                QJsonObject objJson = jsonFile.object();

                bool fullMatch = false;
                for(int i = 0; i < col.size(); i++){
                    QString colStr = col.at(i).toString();
                    QString typeCol = objTypesFromTypes.value(col.at(i).toString()).toString();

                    if (typeCol == "int" || typeCol == "double"){
                        double valueCol = objJson[colStr].toDouble();
                        double valueWhereObj = objWhere[colStr].toDouble();

                        if(valueCol == valueWhereObj){
                            fullMatch = true;
                        } else {
                            fullMatch = false;
                            break;
                        }

                    } else if (typeCol == "bool") {
                        bool valueCol = objJson[colStr].toBool();
                        bool valueWhereObj = objWhere[colStr].toBool();

                        if(valueCol == valueWhereObj){
                            fullMatch = true;
                        } else {
                            fullMatch = false;
                            break;
                        }

                    } else if (typeCol == "string") {
                        QString valueCol = objJson[colStr].toString();
                        QString valueWhereObj = objWhere[colStr].toString();

                        if(valueCol == valueWhereObj){
                            fullMatch = true;
                        } else {
                            fullMatch = false;
                            break;
                        }
                    }

                }

                if(fullMatch){
                    QFile::remove(dir->absolutePath() + "/" + entry);
                    countRemoveFile++;
                }

            }

            QString outputMessage = QString::number(countRemoveFile) + " entries deleted";


            QByteArray resp = jsonResponse(outputMessage);
            sendToClient(pClientSocket, resp); // отправляем структуру
            *conOutput << QTime::currentTime().toString() << " Client (" << pClientSocket->peerAddress().toString() << ") : " << "Deleted " + QString::number(countRemoveFile) + " entries.\n" << Qt::flush;
            dir->cdUp();


        } else if (proc.value("type").toString() == "CHANGE"){
            QString tableName = proc.value("name").toString();

            if (!dir->exists(tableName)){
                //откидываем сообщении о несуществовании таблицы
                QByteArray responseMessage = jsonResponse("The " + tableName + " table does not exist!");
                sendToClient(pClientSocket, responseMessage);
                m_NextBlockSize = 0;
                break;
            }

            int countChangeFile = 0;    // счетчик измененных файлов
            QJsonObject objWhere = proc.value("where").toObject();
            QJsonObject objSet = proc.value("set").toObject();
            QJsonArray colWhere = proc.value("columnsWhere").toArray();
            QJsonArray colSet = proc.value("columnsSet").toArray();
            dir->cd(tableName);

            QFile typesFile(dir->absolutePath() + "/types.entry");
            if(!typesFile.open(QIODevice::ReadOnly | QIODevice::Text)){
                // ошибка
                *conOutput << "File not open!" << Qt::endl << Qt::flush;
            }
            QJsonDocument jsonTypes = QJsonDocument::fromJson(typesFile.readAll());
            typesFile.close();
            QJsonObject objJson = jsonTypes.object();
            QJsonArray columnsFromTypes = objJson.value("columns").toArray();
            QJsonObject objTypesFromTypes = objJson.value("types").toObject();

            QStringList listEntry = dir->entryList(QDir::Files);
            foreach (QString entry, listEntry){
                if(entry == "types.entry"){
                    continue;
                }

                // тут нужна проверка на readLock
                QFile entryFile(dir->absolutePath() + "/" + entry);
                if(!entryFile.open(QIODevice::ReadOnly | QIODevice::Text)){
                    *conOutput << "File not open!" << Qt::endl << Qt::flush;
                }

                QJsonDocument jsonFile = QJsonDocument::fromJson(entryFile.readAll());
                entryFile.close();
                QJsonObject objJson = jsonFile.object();

                bool fullMatch = false;
                for(int i = 0; i < colWhere.size(); i++){
                    QString colStr = colWhere.at(i).toString();
                    QString typeCol = objTypesFromTypes.value(colWhere.at(i).toString()).toString();

                    if (typeCol == "int" || typeCol == "double"){
                        double valueCol = objJson[colStr].toDouble();
                        double valueWhereObj = objWhere[colStr].toDouble();

                        if(valueCol == valueWhereObj){
                            fullMatch = true;
                        } else {
                            fullMatch = false;
                            break;
                        }

                    } else if (typeCol == "bool") {
                        bool valueCol = objJson[colStr].toBool();
                        bool valueWhereObj = objWhere[colStr].toBool();

                        if(valueCol == valueWhereObj){
                            fullMatch = true;
                        } else {
                            fullMatch = false;
                            break;
                        }

                    } else if (typeCol == "string") {
                        QString valueCol = objJson[colStr].toString();
                        QString valueWhereObj = objWhere[colStr].toString();

                        if(valueCol == valueWhereObj){
                            fullMatch = true;
                        } else {
                            fullMatch = false;
                            break;
                        }
                    }

                }

                if(fullMatch){
                    for(int i = 0; i < colSet.size(); i++){
                        QString colStr = colSet.at(i).toString();
                        QString typeCol = objTypesFromTypes.value(colSet.at(i).toString()).toString();

                        if (typeCol == "int" || typeCol == "double"){
                            //double valueCol = objJson[colStr].toDouble();
                            double valueSetObj = objSet[colStr].toDouble();

                            objJson.erase(objJson.find(colStr)); // удаление старового значения
                            objJson.insert(colStr, valueSetObj); // добавление нового



                        } else if (typeCol == "bool") {
                            //bool valueCol = objJson[colStr].toBool();
                            bool valueSetObj = objSet[colStr].toBool();

                            objJson.erase(objJson.find(colStr)); // удаление старового значения
                            objJson.insert(colStr, valueSetObj); // добавление нового

                        } else if (typeCol == "string") {
                            //QString valueCol = objJson[colStr].toString();
                            QString valueSetObj = objSet[colStr].toString();

                            objJson.erase(objJson.find(colStr)); // удаление старового значения
                            objJson.insert(colStr, valueSetObj); // добавление нового
                        }

                    }

                    if(!entryFile.open(QIODevice::WriteOnly)){
                        *conOutput << "File not open!" << Qt::endl << Qt::flush;
                    }

                    jsonFile.setObject(objJson);
                    entryFile.write(jsonFile.toJson());
                    entryFile.close();

                    countChangeFile++;

                }

            }

            QString outputMessage = QString::number(countChangeFile) + " entries changed";


            QByteArray resp = jsonResponse(outputMessage);
            sendToClient(pClientSocket, resp); // отправляем структуру
            *conOutput << QTime::currentTime().toString() << " Client (" << pClientSocket->peerAddress().toString() << ") : " << "Changed " + QString::number(countChangeFile) + " entries.\n" << Qt::flush;
            dir->cdUp();

        }

        //QString strMessage = QTime::currentTime().toString() + " " + str + '\n';
        //*conOutput << "Client (" << pClientSocket->localAddress().toString() << "): " << strMessage << '\n' << Qt::flush;
        m_NextBlockSize = 0;
        
        //sendToClient(pClientSocket, "Server echo: " + str + "\n");
    }
}


void ServerData::sendToClient(QTcpSocket* pSocket, QByteArray& response){
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);

    out << quint16(0) << response;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    pSocket->write(arrBlock);
}

void ServerData::slotDisconnected(){
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();

    *conOutput << QTime::currentTime().toString() << " Client (" << pClientSocket->peerAddress().toString() << ") disconnected.\n" << Qt::flush;

    pClientSocket->deleteLater();

}




QByteArray ServerData::jsonResponse(QString message){
    QJsonObject respObj;
    respObj.insert("type", "message");
    respObj.insert("response", message);
    QJsonDocument responseJson(respObj);
    return responseJson.toJson();
}

//QByteArray ServerData::jsonResponse(QString type, QByteArray &information){

//}

