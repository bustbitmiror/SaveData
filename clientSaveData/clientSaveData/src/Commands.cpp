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

//--------------- Commands----------------

QByteArray ClientData::createTable(){
    std::system("cls");
    QJsonObject objCreate;              // создаем объект QJson запроса
    objCreate.insert("type", "CREATE"); // тип запроса CREATE

    //QMap<QString, QString> columnsAndTypes;
    QString nameTable;
    QString namesColumns;

    *out << "Table name: " << Qt::flush;

    nameTable = in->readLine();   // читаем название таблицы
    in->flush();

    objCreate.insert("name", nameTable);  // добавляем его в json запрос

    *out << "Column names (separated by a space): " << Qt::flush;

    namesColumns = in->readLine();      // читаем названия столбцов
    in->flush();
    QStringList column = namesColumns.split(u' ', Qt::SkipEmptyParts);

    QStringList typesColumns;
    bool matchSizeAndType = false;
    while(matchSizeAndType != true){
        *out << "Column type (separated by a space): " << Qt::flush;

        typesColumns = (in->readLine()).split(u' ', Qt::SkipEmptyParts); // читаем типы наших колонок
        in->flush();

        //*out << "size: " << typesColumns.size() << Qt::endl << Qt::flush;
        if(typesColumns.size() != column.size()){
            *out << "Unequal number of columns and types!\n" << Qt::flush;
            matchSizeAndType = false;
        } else {
            for (int i = 0; i < typesColumns.size(); i++){
                //*out << typesColumns.at(i) << " " << Qt::flush;
                matchSizeAndType = true;
                if(!typesCol.contains(typesColumns.at(i))){
                    *out << "Does not match the types!\n" << Qt::flush;
                    matchSizeAndType = false;
                    break;
                }
            }

            if (matchSizeAndType){
                break;
            }

        }
    }

    QJsonArray columns;
    QJsonArray types;

    for (int i = 0; i < column.size(); i++){  // добавляем в массивы QJson запроса
        columns.append(column.at(i));
        types.append(typesColumns.at(i));
    }

    objCreate.insert("columns", columns);
    objCreate.insert("typesColumns", types);

    QJsonDocument jsonRequest(objCreate);

    //*out << "The request has been formed.\n" << Qt::flush;

//    *out << nameTable << '\n';
//    out->flush();



    //std::cin.get();

    return jsonRequest.toJson();
}


QByteArray ClientData::viewsTable(){
    std::system("cls");
    QJsonObject objViews;              // создаем объект QJson запроса
    objViews.insert("type", "VIEWS");  //  тип запроса VIEWS

    QJsonDocument jsonRequest(objViews);
    return jsonRequest.toJson();
}


QByteArray ClientData::insertDataInTable(){
    std::system("cls");
    QJsonObject objInsert;              // создаем объект QJson запроса
    objInsert.insert("type", "INSERT");  //  тип запроса INSERT

    QString nameTable;
    *out << "Table name: " << Qt::flush;
    nameTable = in->readLine();   // читаем название таблицы
    in->flush();

    // ----------Information request-----------
    QJsonObject objInfo;
    objInfo.insert("type", "INFORMATION");
    objInfo.insert("name", nameTable);
    QJsonDocument jsonInfo(objInfo);
    QByteArray infoMessage = jsonInfo.toJson();
    sendMessageToServer(infoMessage);
    m_tcpSocket->waitForReadyRead();
    jsonInfo = QJsonDocument::fromJson(*bufferResponse);
    bufferResponse->clear();
    objInfo = jsonInfo.object();
    if (objInfo.value("type").toString() == "information"){
        // продолжаем...

    } else if (objInfo.value("type").toString() == "message"){
        *out << objInfo.value("response").toString() << Qt::flush;
        return QByteArray();
    }
    // ---------------------------------------

    objInsert.insert("name", nameTable);  // добавляем название в объект запроса

    QStringList column;
    QStringList valuesColumns;

    QJsonArray col = objInfo.value("columns").toArray();
    QJsonObject types = objInfo.value("types").toObject();

    *out << "Existing columns of the " << nameTable << " table: ";
    for (int i = 0; i < col.size(); i++){
        *out << col.at(i).toString() << " ";
    }
    *out << '\n';
    *out << "Types columns:\n";
    for (int i = 0; i < col.size(); i++){
        *out << "\t" << col.at(i).toString() << " : " << types[col.at(i).toString()].toString() << '\n';
    }
    *out << Qt::flush;

    bool matchSize = false;
    while(matchSize != true){
        *out << "Column names to insert (separated by a space): " << Qt::flush;

        // читаем названия столбцов
        column = (in->readLine()).split(u' ', Qt::SkipEmptyParts);
        in->flush();


        // проверка на соответствие с существующей таблицей
        bool match = true;
        for(int i = 0; i < column.size(); i++){
            if(!col.contains(column.at(i))){
                *out << "Unknown column\n" << Qt::flush;
                match = false;
                break;
            }
        }

        if(!match){
            continue;
        }


        //QStringList valuesColumns;
        *out << "Column value to insert (separated by a space): " << Qt::flush;

        valuesColumns = (in->readLine()).split(u' ', Qt::SkipEmptyParts); // читаем значения наших колонок
        in->flush();

        if(valuesColumns.size() != column.size()){
            *out << "Unequal number of columns and values!\n" << Qt::flush;
            matchSize = false;
        } else {
            matchSize = true;
        }
    }

    QJsonArray columns;
    QJsonArray values;

    for (int i = 0; i < column.size(); i++){  // добавляем в массивы QJson запроса
        columns.append(column.at(i));

        if (types[column.at(i)].toString() == "int" || types[column.at(i)].toString() == "double"){
            if (valuesColumns.at(i).size() > INT_SIZE){
                *out << "The int/double type must be no more than 11 characters!\n" << Qt::flush;
                return QByteArray();
            }
            values.append(valuesColumns.at(i).toDouble());
        } else if (types[column.at(i)].toString() == "string"){
            if (valuesColumns.at(i).size() > STRING_SIZE){
                *out << "The string type must be no more than 40 characters!\n" << Qt::flush;
                return QByteArray();
            }
            values.append(valuesColumns.at(i));
        } else if (types[column.at(i)].toString() == "bool"){
            if (valuesColumns.at(i).size() > BOOL_SIZE){
                *out << "The bool type must be no more than 5 characters!\n" << Qt::flush;
                return QByteArray();
            }
            values.append(valuesColumns.at(i));
        }

    }

    objInsert.insert("columns", columns);
    objInsert.insert("values", values);

    QJsonDocument jsonRequest(objInsert);


    return jsonRequest.toJson();
}


QByteArray ClientData::viewsStruct(){
    std::system("cls");
    QJsonObject objViewsStruct;
    objViewsStruct.insert("type", "VIEWSSTRUCT");

    QString nameTable;
    *out << "Table name: " << Qt::flush;
    nameTable = in->readLine();   // читаем название таблицы
    in->flush();

    objViewsStruct.insert("name", nameTable);  // добавляем название в объект запроса

    QJsonDocument jsonRequest(objViewsStruct);

    return jsonRequest.toJson();
}

QByteArray ClientData::readData(){
    std::system("cls");
    QJsonObject objRead;              // создаем объект QJson запроса
    objRead.insert("type", "SELECT");  //  тип запроса SELECT

    QString nameTable;
    *out << "Table name: " << Qt::flush;
    nameTable = in->readLine();   // читаем название таблицы
    in->flush();

    objRead.insert("name", nameTable);  // добавляем его в json запрос

    *out << "Column names (separated by a space): " << Qt::flush;

    QStringList column = (in->readLine()).split(u' ', Qt::SkipEmptyParts);
    in->flush();

    QJsonArray columns;

    for (int i = 0; i < column.size(); i++){  // добавляем в массивы QJson запроса
        columns.append(column.at(i));
    }

    objRead.insert("columns", columns);

    QJsonDocument jsonRequest(objRead);


    return jsonRequest.toJson();
}


QByteArray ClientData::deleteData(){
    std::system("cls");
    QJsonObject objDelete;              // создаем объект QJson запроса
    objDelete.insert("type", "DELETE");  //  тип запроса DELETE

    QString nameTable;
    *out << "Table name: " << Qt::flush;
    nameTable = in->readLine();   // читаем название таблицы
    in->flush();


    // ----------Information request-----------
    QJsonObject objInfo;
    objInfo.insert("type", "INFORMATION");
    objInfo.insert("name", nameTable);
    QJsonDocument jsonInfo(objInfo);
    QByteArray infoMessage = jsonInfo.toJson();
    sendMessageToServer(infoMessage);
    m_tcpSocket->waitForReadyRead();
    jsonInfo = QJsonDocument::fromJson(*bufferResponse);
    bufferResponse->clear();
    objInfo = jsonInfo.object();
    if (objInfo.value("type").toString() == "information"){
        // продолжаем...

    } else if (objInfo.value("type").toString() == "message"){
        *out << objInfo.value("response").toString() << Qt::flush;
        return QByteArray();
    }
    // ---------------------------------------

    objDelete.insert("name", nameTable);  // добавляем название в объект запроса

    QStringList column;
    QStringList valuesColumns;

    QJsonArray col = objInfo.value("columns").toArray();
    QJsonObject types = objInfo.value("types").toObject();

    *out << "Existing columns of the " << nameTable << " table: ";
    for (int i = 0; i < col.size(); i++){
        *out << col.at(i).toString() << " ";
    }
    *out << '\n';
    *out << "Types columns:\n";
    for (int i = 0; i < col.size(); i++){
        *out << "\t" << col.at(i).toString() << " : " << types[col.at(i).toString()].toString() << '\n';
    }
    *out << Qt::flush;


    bool deleteTable;
    while(true){
        *out << "Do you want to delete a table? (yes/no) -> " << Qt::flush;
        QString del = in->readLine();
        in->flush();

        if (del.toLower() == "yes"){
            deleteTable = true;
            break;
        } else if (del.toLower() == "no"){
            deleteTable = false;
            break;
        } else {
            *out << "The answer is entered incorrectly.\n" << Qt::flush;
        }
    }


    if (deleteTable){
        objDelete.insert("deleteTable", "yes");
    } else {
        objDelete.insert("deleteTable", "no");

        bool matchSize = false;
        while(matchSize != true){
            *out << "Column names for deleting a record/entries (separated by a space): " << Qt::flush;

            // читаем названия столбцов
            column = (in->readLine()).split(u' ', Qt::SkipEmptyParts);
            in->flush();


            // проверка на соответствие с существующей таблицей
            bool match = true;
            for(int i = 0; i < column.size(); i++){
                if(!col.contains(column.at(i))){
                    *out << "Unknown column\n" << Qt::flush;
                    match = false;
                    break;
                }
            }

            if(!match){
                continue;
            }

            for (int i = 0; i < column.size(); i++){
                if (column.count(column.at(i)) > 1){
                    match = false;
                    *out << "The column met more than 1 time! You can't do that.\n" << Qt::flush;
                    break;
                }
            }

            if(!match){
                continue;
            }

            //QStringList valuesColumns;
            *out << "The values of these columns (separated by a space): " << Qt::flush;

            valuesColumns = (in->readLine()).split(u' ', Qt::SkipEmptyParts); // читаем значения наших колонок
            in->flush();

            if(valuesColumns.size() != column.size()){
                *out << "Unequal number of columns and values!\n" << Qt::flush;
                matchSize = false;
            } else {
                matchSize = true;
            }
        }

        QJsonObject objWhere;  // для хранения ключ значений поиска
        QJsonArray columns;

        for(int i = 0; i < column.size(); i++){
            columns.append(column.at(i));
            if (types[column.at(i)].toString() == "int" || types[column.at(i)].toString() == "double"){

    //            if(objWhere.constFind(column.at(i)) == objWhere.constEnd()){
    //                columns.append(column.at(i));
    //                objWhere.insert(column.at(i), valuesColumns.at(i).toDouble());
    //            }
                objWhere.insert(column.at(i), valuesColumns.at(i).toDouble());

            } else if (types[column.at(i)].toString() == "string"){

    //            if(objWhere.constFind(column.at(i)) == objWhere.constEnd()){
    //                columns.append(column.at(i));
    //                objWhere.insert(column.at(i), valuesColumns.at(i));
    //            }
                objWhere.insert(column.at(i), valuesColumns.at(i));
            } else if (types[column.at(i)].toString() == "bool"){

    //            if(objWhere.constFind(column.at(i)) == objWhere.constEnd()){
    //                columns.append(column.at(i));
    //                objWhere.insert(column.at(i), valuesColumns.at(i));
    //            }
                objWhere.insert(column.at(i), valuesColumns.at(i));

            }
        }

        objDelete.insert("columns", columns);
        objDelete.insert("where", objWhere);
    }




    QJsonDocument jsonRequest(objDelete);


    return jsonRequest.toJson();

}

QByteArray ClientData::changeData(){
    std::system("cls");
    QJsonObject objChange;              // создаем объект QJson запроса
    objChange.insert("type", "CHANGE");  //  тип запроса CHANGE

    QString nameTable;
    *out << "Table name: " << Qt::flush;
    nameTable = in->readLine();   // читаем название таблицы
    in->flush();


    // ----------Information request-----------
    QJsonObject objInfo;
    objInfo.insert("type", "INFORMATION");
    objInfo.insert("name", nameTable);
    QJsonDocument jsonInfo(objInfo);
    QByteArray infoMessage = jsonInfo.toJson();
    sendMessageToServer(infoMessage);
    m_tcpSocket->waitForReadyRead();
    jsonInfo = QJsonDocument::fromJson(*bufferResponse);
    bufferResponse->clear();
    objInfo = jsonInfo.object();
    if (objInfo.value("type").toString() == "information"){
        // продолжаем...

    } else if (objInfo.value("type").toString() == "message"){
        *out << objInfo.value("response").toString() << Qt::flush;
        return QByteArray();
    }
    // ---------------------------------------

    objChange.insert("name", nameTable);  // добавляем название в объект запроса



    QJsonArray col = objInfo.value("columns").toArray();
    QJsonObject types = objInfo.value("types").toObject();

    *out << "Existing columns of the " << nameTable << " table: ";
    for (int i = 0; i < col.size(); i++){
        *out << col.at(i).toString() << " ";
    }
    *out << '\n';
    *out << "Types columns:\n";
    for (int i = 0; i < col.size(); i++){
        *out << "\t" << col.at(i).toString() << " : " << types[col.at(i).toString()].toString() << '\n';
    }
    *out << Qt::flush;


    QStringList columnSet;
    QStringList valuesColumnsSet;
    // for set
    bool matchSizeSet = false;
    while(matchSizeSet != true){
        *out << "Column names to change (separated by a space) (set): " << Qt::flush;

        // читаем названия столбцов
        columnSet = (in->readLine()).split(u' ', Qt::SkipEmptyParts);
        in->flush();


        // проверка на соответствие с существующей таблицей
        bool match = true;
        for(int i = 0; i < columnSet.size(); i++){
            if(!col.contains(columnSet.at(i))){
                *out << "Unknown column\n" << Qt::flush;
                match = false;
                break;
            }
        }

        if(!match){
            continue;
        }

        for (int i = 0; i < columnSet.size(); i++){
            if (columnSet.count(columnSet.at(i)) > 1){
                match = false;
                *out << "The column met more than 1 time! You can't do that.\n" << Qt::flush;
                break;
            }
        }

        if(!match){
            continue;
        }

        //QStringList valuesColumns;
        *out << "The values of these columns (separated by a space): " << Qt::flush;

        valuesColumnsSet = (in->readLine()).split(u' ', Qt::SkipEmptyParts); // читаем значения наших колонок
        in->flush();

        if(valuesColumnsSet.size() != columnSet.size()){
            *out << "Unequal number of columns and values!\n" << Qt::flush;
            matchSizeSet = false;
        } else {
            matchSizeSet = true;
        }
    }

    QStringList columnWhere;
    QStringList valueWhere;

    // for where
    bool matchSizeWhere = false;
    while(matchSizeWhere != true){
        *out << "Column names with existing values (separated by a space) (where): " << Qt::flush;

        // читаем названия столбцов
        columnWhere = (in->readLine()).split(u' ', Qt::SkipEmptyParts);
        in->flush();


        // проверка на соответствие с существующей таблицей
        bool match = true;
        for(int i = 0; i < columnWhere.size(); i++){
            if(!col.contains(columnWhere.at(i))){
                *out << "Unknown column\n" << Qt::flush;
                match = false;
                break;
            }
        }

        if(!match){
            continue;
        }

        for (int i = 0; i < columnWhere.size(); i++){
            if (columnWhere.count(columnWhere.at(i)) > 1){
                match = false;
                *out << "The column met more than 1 time! You can't do that.\n" << Qt::flush;
                break;
            }
        }

        if(!match){
            continue;
        }

        //QStringList valuesColumns;
        *out << "The values of these columns (separated by a space): " << Qt::flush;

        valueWhere = (in->readLine()).split(u' ', Qt::SkipEmptyParts); // читаем значения наших колонок
        in->flush();

        if(valueWhere.size() != columnWhere.size()){
            *out << "Unequal number of columns and values!\n" << Qt::flush;
            matchSizeWhere = false;
        } else {
            matchSizeWhere = true;
        }
    }





    QJsonObject objWhere;  // для хранения ключ значений поиска
    QJsonObject objSet;
    QJsonArray columnsSet;
    QJsonArray columnsWhere;

    //set
    for(int i = 0; i < columnSet.size(); i++){
        columnsSet.append(columnSet.at(i));
        if (types[columnSet.at(i)].toString() == "int" || types[columnSet.at(i)].toString() == "double"){

//            if(objWhere.constFind(column.at(i)) == objWhere.constEnd()){
//                columns.append(column.at(i));
//                objWhere.insert(column.at(i), valuesColumns.at(i).toDouble());
//            }
            objSet.insert(columnSet.at(i), valuesColumnsSet.at(i).toDouble());

        } else if (types[columnSet.at(i)].toString() == "string"){

//            if(objWhere.constFind(column.at(i)) == objWhere.constEnd()){
//                columns.append(column.at(i));
//                objWhere.insert(column.at(i), valuesColumns.at(i));
//            }
            objSet.insert(columnSet.at(i), valuesColumnsSet.at(i));
        } else if (types[columnSet.at(i)].toString() == "bool"){

//            if(objWhere.constFind(column.at(i)) == objWhere.constEnd()){
//                columns.append(column.at(i));
//                objWhere.insert(column.at(i), valuesColumns.at(i));
//            }
            objSet.insert(columnSet.at(i), valuesColumnsSet.at(i));

        }
    }

    // where
    for(int i = 0; i < columnWhere.size(); i++){
        columnsWhere.append(columnWhere.at(i));
        if (types[columnWhere.at(i)].toString() == "int" || types[columnWhere.at(i)].toString() == "double"){

//            if(objWhere.constFind(column.at(i)) == objWhere.constEnd()){
//                columns.append(column.at(i));
//                objWhere.insert(column.at(i), valuesColumns.at(i).toDouble());
//            }
            objWhere.insert(columnWhere.at(i), valueWhere.at(i).toDouble());

        } else if (types[columnWhere.at(i)].toString() == "string"){

//            if(objWhere.constFind(column.at(i)) == objWhere.constEnd()){
//                columns.append(column.at(i));
//                objWhere.insert(column.at(i), valuesColumns.at(i));
//            }
            objWhere.insert(columnWhere.at(i), valueWhere.at(i));
        } else if (types[columnWhere.at(i)].toString() == "bool"){

//            if(objWhere.constFind(column.at(i)) == objWhere.constEnd()){
//                columns.append(column.at(i));
//                objWhere.insert(column.at(i), valuesColumns.at(i));
//            }
            objWhere.insert(columnWhere.at(i), valueWhere.at(i));

        }
    }





    objChange.insert("columnsSet", columnsSet);
    objChange.insert("columnsWhere", columnsWhere);
    objChange.insert("set", objSet);
    objChange.insert("where", objWhere);
    QJsonDocument jsonRequest(objChange);


    return jsonRequest.toJson();

}
