#ifndef CFOLDER_H
#define CFOLDER_H

#include <QSqlDatabase>
#include <QString>
#include "ctreeitem.h"
#include <QMetaType>

class CFolder;

class CFolder : public CTreeItem
{
public:
    CFolder();
    CFolder(QSqlQuery & query);

    void set_title(QString title);
    void set_parent_id(int parent_id);
    bool insert_to_db();

    int get_id() const;
    int get_parent_id();
    void set_id(int id);
    void set_created_at (int created_at);
    void set_deleted_id (int deleted_id);

    QString get_title() const;

protected:
    QString _title;
    int _id;
    int _created_at;
    int _parent_id;
    int _deleted_id;
};

Q_DECLARE_METATYPE(CFolder*)

#endif // CFOLDER_H
