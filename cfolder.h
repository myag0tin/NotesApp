#ifndef CFOLDER_H
#define CFOLDER_H

#include <QSqlDatabase>
#include <QString>
#include "cdatabase.h"

class CFolder
{
public:
    CFolder(CDataBase * db);

    void set_title(QString title);
    void set_parent(CFolder * parent);
    bool insert_to_db();
    // todo
    int get_id() const;

protected:
    CDataBase * _db;
    QString _title;
    CFolder * _parent;
    int _id;
};

#endif // CFOLDER_H
