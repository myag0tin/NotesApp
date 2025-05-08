#ifndef CNOTE_H
#define CNOTE_H

#include <QMetaType>
#include <QSqlDatabase>
#include <QString>
#include "cfolder.h"
#include "ctreeitem.h"


class CNote : public CTreeItem
{
public:
    CNote();
    CNote(QSqlQuery & query);

    void set_content(QString content);
    void set_title(QString title);
    void set_parent(CFolder * parent);
    bool insert_to_db();

    int get_id() const;
    QString get_title() const;
    QString get_content() const;
    CFolder* get_parent() const;
    void set_id(int id);
    void set_created_at(int created_at);
    void set_parent_id(int parent_id);
    void set_deleted_id(int deleted_id);

    int get_parent_id() const;

protected:

     QString _content;
     QString _title;
     CFolder * _parent;
     int _id;
     int _created_at;
     int _parent_id;
     int _deleted_id;
};

inline int CNote::get_parent_id() const { return _parent_id; }

Q_DECLARE_METATYPE(CNote*)

#endif // CNOTE_H
