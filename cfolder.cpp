#include "cfolder.h"
#include "cdatabase.h"
#include <QDateTime>
#include <QSqlQuery>

#include<iostream>

CFolder::CFolder() : _id(-1), _parent_id(0), _created_at(0), _deleted_id(0)
{
}

CFolder::CFolder(QSqlQuery & query)
{
    set_id(query.value("id").toInt());
    set_title(query.value("title").toString());
    set_created_at(query.value("created_at").toInt());
    set_parent_id(query.value("parent_id").toInt());
    set_deleted_id(query.value("deleted_id").toInt());
}

void CFolder::set_deleted_id(int deleted_id)
{
    _deleted_id = deleted_id;
}

void CFolder::set_parent_id(int parent_id)
{
    _parent_id = parent_id;
}


int CFolder::get_parent_id()
{
    return _parent_id;
}

void CFolder::set_created_at(int created_at)
{
    _created_at = created_at;
}

void CFolder::set_title(QString title)
{
    _title = title;
}


void CFolder::set_id(int id)
{
    _id = id;
}

QString CFolder::get_title() const
{
    return _title;
}



bool CFolder::insert_to_db()
{
    return true;
    //return _db->insert(this);
}

int CFolder::get_id() const
{
    return this->_id;
}
