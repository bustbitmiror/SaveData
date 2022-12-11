#ifndef __STREAM_TABLE_H
#define __STREAM_TABLE_H

//#undef max
//#undef min

//#include <string>
//#include <vector>
//#include <algorithm>
//#include <iostream>
//#include <QTextStream>
//#include <cstring>
//#include <QVector>
//#include <QString>
////стратегия формирования единой таблицы
//#define CRLF "\n"


////стратегия построчной выгрузки таблицы
////#define CRLF Qt::endl


//class StreamTable {
//public:
//    QTextStream &os_;

//    StreamTable(QTextStream &os, char delimRow = ' ', char delimCol = ' ') :
//        borderExtOn_(true),     // есть ли внешние границы таблицы
//        delimRowOn_(true),      // есть ли разделитель строк
//        delimRow_(delimRow),   // символ разделителя строк
//        delimColOn_(true),    // есть ли разделитель столбцов
//        delimCol_(delimCol),  // символ разделителя столбцов
//        os_(os),
//        colIndex_(0),        // индекс колонки текущей
//        firstCell_(1) {}

//    virtual ~StreamTable() {}

//    virtual QTextStream &os() const {
//        return os_;
//    }

//    //отображать внешние границы?
//    void MakeBorderExt(bool on) {
//        borderExtOn_ = on;
//    }

//    //символ разделителя строк
//    void SetDelimRow(bool delimOn, char delimRow = ' ') {
//        delimRowOn_ = delimOn;
//        if (delimRowOn_)
//            delimRow_ = delimRow;
//        else if (!delimColOn_)
//            MakeBorderExt(false);    // выключить внешние границы
//    }

//    //символ разделителя столбцов
//    void SetDelimCol(bool delimOn, char delimCol = ' ') {
//        delimColOn_ = delimOn;
//        if (delimColOn_)
//            delimCol_ = delimCol;
//        else if (!delimRowOn_)
//            MakeBorderExt(false);   // выключить внешние границы
//    }

//    int AddCol(int colWidth, bool visible = true) {
//        colWidth_.push_back(colWidth);      // длина колонки
//        visible_.push_back(visible);        // видимость колонки
//        return colWidth_.back();
//    }

//    void SetVisible(int col, bool flg) {  // видимости колонки по i = col
//        visible_[col - 1] = flg;
//    }

//    // количество colCount с шириной colWidth - одинаковой длины колонки
//    void SetCols(int colCount, int colWidth = 0) {
//        Clear();

//        for (int ic = 0; ic < colCount; ic++) {
//            AddCol(colWidth);
//        }
//    }

//    // отчистка колонок
//    virtual void Clear() {
//        colWidth_.clear();
//        visible_.clear();
//        colIndex_ = 0;
//        firstCell_ = 1;
//    }

//    // добавить пустую строку
//    void AddEmptyRow() {
//        for (int ic = 0; ic < (int)colWidth_.size(); ic++) {
//            *this << "";
//        }
//    }

//    template <typename T> StreamTable &operator << (const T &obj) {
//        Push(obj);
//        return *this;
//    }

//    StreamTable &operator << (const QString &s) {
//        colWidth_[colIndex_] = std::max(colWidth_[colIndex_], (int)s.size() + 1);
//        Push(s);
//        return *this;
//    }

//    StreamTable &operator << (const char *s) {
//        colWidth_[colIndex_] = std::max(colWidth_[colIndex_], (int)strlen(s) + 1);
//        Push(s);
//        return *this;
//    }

//protected:
//    int colIndex_;  // индекс колонки

//private:
//    bool borderExtOn_;  // есть ли внешняя граница
//    bool delimRowOn_;   // есть ли разделитель по строке
//    char delimRow_;     // символ разделителя по строке

//    bool delimColOn_;   // есть ли разделитель по колонке
//    char delimCol_;     // символ разделителя по солонке

//    QVector<int> colWidth_; // массив длин колонок
//    bool firstCell_;  // первая ли строка вывода
//    QVector<int> visible_;  // видимость каждой колонки

//    // добавление данных
//    template <typename T>
//    void Push(const T &obj) {
//        // проверка строки вывода (первая ли)
//        if (firstCell_) {
//            if (borderExtOn_) // есть ли внешнаяя граница
//                MakeRowBorder(); // делаем внешнюю границы

//            firstCell_ = 0;
//        }

//        // проверка видимости колонки
//        if (visible_[colIndex_]) {
//            DelimCol();     // делитель колонки

//            //os_.width(colWidth_[colIndex_]); // заполняем шириной колонки
//            //os_.fill(' ');                   // заполняем пробелами
//            os_.setFieldWidth(colWidth_[colIndex_]);
//            os_.setPadChar(' ');
//            //os_.setFieldAlignment(QTextStream::AlignLeft);
//            os_ << /*std::setiosflags(std::ios::left) << */obj; // ставим наши данные
//        }

//        // если ли индекс след.колонки равна количеству всех колонок
//        if (++colIndex_ == (int)colWidth_.size()) {
//            DelimCol(); // ставим разделитель колонок

//            if (delimRowOn_) // есль ли разделитель строк
//                MakeRowBorder();   // ставим разделитель строки
//            else
//                os_ << CRLF;       // или же просто перенос строки

//            colIndex_ = 0;         // ставим нулевой индекс
//        }
//    }

//    // делаем границу по строке
//    void MakeRowBorder() {
//        os_ << CRLF; // символ /n - следующей строки
//        DelimCol();  // делитель колонки

//        // проходим по всем колонкам и по ширине каждого заполняем границы
//        int ic;
//        for (ic = 0; ic < (int)colWidth_.size(); ic++) {
//            if (visible_[ic]) {
//                //os_.width(colWidth_[ic] + 1); // делаем ширину
//                //os_.fill(delimRow_);          // заполняем это символом разделителя
//                os_.setFieldWidth(colWidth_[ic] + 1);
//                os_.setPadChar(delimRow_);
//                //os_.setFieldAlignment(QTextStream::AlignLeft);
//                DelimCol();                   // делитель колонки
//            }
//        }
//        os_ << CRLF;
//    }

//    // ставим делитель колонки
//    void DelimCol() {
//        if (delimColOn_ && (borderExtOn_ || colIndex_))
//            os_ << delimCol_;
//        else
//            os_ << ' ';
//    }

//    //запрет на копирование
//    StreamTable &operator = (const StreamTable &);
//};


#undef max
#undef min

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

//стратегия формирования единой таблицы
#define CRLF "\n"

//стратегия построчной выгрузки таблицы
//#define CRLF std::endl

/**
* Прямоугольная таблица с разделителями строк и столбцов
* Синтаксис как у потоков C++
*/
class StreamTable {
public:
    std::ostream &os_;

    StreamTable(std::ostream &os, char delimRow = ' ', char delimCol = ' ') :
        borderExtOn_(true),
        delimRowOn_(true),
        delimRow_(delimRow),
        delimColOn_(true),
        delimCol_(delimCol),
        os_(os),
        colIndex_(0),
        firstCell_(1) {}

    virtual ~StreamTable() {}

    virtual std::ostream &os() const {
        return os_;
    }

    //отображать внешние границы?
    void MakeBorderExt(bool on) {
        borderExtOn_ = on;
    }

    //символ разделителя строк
    void SetDelimRow(bool delimOn, char delimRow = ' ') {
        delimRowOn_ = delimOn;
        if (delimRowOn_)
            delimRow_ = delimRow;
        else if (!delimColOn_)
            MakeBorderExt(false);
    }

    //символ разделителя столбцов
    void SetDelimCol(bool delimOn, char delimCol = ' ') {
        delimColOn_ = delimOn;
        if (delimColOn_)
            delimCol_ = delimCol;
        else if (!delimRowOn_)
            MakeBorderExt(false);
    }

    int AddCol(int colWidth, bool visible = true) {
        colWidth_.push_back(colWidth);
        visible_.push_back(visible);
        return colWidth_.back();
    }

    void SetVisible(int col, bool flg) {
        visible_[col - 1] = flg;
    }

    void SetCols(int colCount, int colWidth = 0) {
        Clear();

        for (int ic = 0; ic < colCount; ic++) {
            AddCol(colWidth);
        }
    }

    virtual void Clear() {
        colWidth_.clear();
        visible_.clear();
        colIndex_ = 0;
        firstCell_ = 1;
    }

    void AddEmptyRow() {
        for (int ic = 0; ic < (int)colWidth_.size(); ic++) {
            *this << "";
        }
    }

    template <typename T> StreamTable &operator << (const T &obj) {
        Push(obj);
        return *this;
    }

    StreamTable &operator << (const std::string &s) {
        colWidth_[colIndex_] = std::max(colWidth_[colIndex_], (int)s.size() + 1);
        Push(s);
        return *this;
    }

    StreamTable &operator << (const char *s) {
        colWidth_[colIndex_] = std::max(colWidth_[colIndex_], (int)strlen(s) + 1);
        Push(s);
        return *this;
    }

protected:
    int colIndex_;

private:
    bool borderExtOn_;
    bool delimRowOn_;
    char delimRow_;

    bool delimColOn_;
    char delimCol_;

    std::vector<int> colWidth_;
    bool firstCell_;
    std::vector<int> visible_;

    template <typename T>
    void Push(const T &obj) {
        if (firstCell_) {
            if (borderExtOn_)
                MakeRowBorder();

            firstCell_ = 0;
        }

        if (visible_[colIndex_]) {
            DelimCol();

            os_.width(colWidth_[colIndex_]);
            os_.fill(' ');
            os_ << /*std::setiosflags(std::ios::left) << */obj;
        }

        if (++colIndex_ == (int)colWidth_.size()) {
            DelimCol();

            if (delimRowOn_)
                MakeRowBorder();
            else
                os_ << CRLF;

            colIndex_ = 0;
        }
    }

    void MakeRowBorder() {
        os_ << CRLF;
        DelimCol();

        int ic;
        for (ic = 0; ic < (int)colWidth_.size(); ic++) {
            if (visible_[ic]) {
                os_.width(colWidth_[ic] + 1);
                os_.fill(delimRow_);
                DelimCol();
            }
        }
        os_ << CRLF;
    }

    void DelimCol() {
        if (delimColOn_ && (borderExtOn_ || colIndex_))
            os_ << delimCol_;
        else
            os_ << ' ';
    }

    //запрет на копирование
    StreamTable &operator = (const StreamTable &);
};





















#endif // __STREAM_TABLE_H
