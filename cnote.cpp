#include "cnote.h"
#include "cdatabase.h"
#include <QDateTime>
#include <QSqlQuery>

CNote::CNote() : _parent(nullptr), _id(-1)
{
}


CNote::CNote(QSqlQuery & query)
{
    set_id(query.value("id").toInt());
    set_title(query.value("title").toString());

    set_content(query.value("content").toString());
    set_created_at(query.value("created_at").toInt());
    set_parent_id(query.value("parent_id").toInt());
    set_deleted_id(query.value("deleted_id").toInt());
}

void CNote::set_deleted_id(int deleted_id)
{
    _deleted_id = deleted_id;
}

void CNote::set_parent_id(int parent_id)
{
    _parent_id = parent_id;
}

void CNote::set_created_at(int created_at)
{
    _created_at = created_at;
}

void CNote::set_content(QString content)
{
    _content = content;
}

void CNote::set_title(QString title)
{
    _title = title;
}

void CNote::set_parent(CFolder * parent)
{
    _parent = parent;
}

void CNote::set_id(int id)
{
    _id = id;
}

QString CNote::get_title() const
{
    return _title;
}

QString CNote::get_content() const
{
    return _content;
}

CFolder* CNote::get_parent() const
{
    return _parent;
}

bool CNote::insert_to_db()
{
    return true;

}

int CNote::get_id() const
{
    return _id;
}
