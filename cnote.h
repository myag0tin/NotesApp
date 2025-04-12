#ifndef CNOTE_H
#define CNOTE_H

#include <QSqlDatabase>
#include <QString>
#include "cfolder.h"
#include "cdatabase.h"

class CNote
{
public:
    CNote(CDataBase * db);

    void set_content(QString content);
    void set_title(QString title);
    void set_parent(CFolder * parent);
    bool insert_to_db();

    int get_id() const;

protected:
     CDataBase * _db;
     QString _content;
     QString _title;
     CFolder * _parent;
     int _id;
};

#endif // CNOTE_H
