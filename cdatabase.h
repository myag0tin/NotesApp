#ifndef CDATABASE_H
#define CDATABASE_H

#include <QSqlDatabase>
#include <QString>

class CDataBase
{
public:
    CDataBase(const QString& path);
    ~CDataBase();

    bool is_open() const;
    //QSqlDatabase* get_db();
    bool insert(CNote * note);
    bool insert(CFolder * folder);


private:
    QSqlDatabase _db;
    QString _connectionName;
};

#endif // CDATABASE_H
