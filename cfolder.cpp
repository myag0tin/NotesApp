#include "cfolder.h"
#include <QDateTime>
#include <QSqlQuery>
#include <QVariant>

CFolder::CFolder(CDataBase * db) : _db(db), _parent(nullptr), _id(-1)
{
}

void CFolder::set_title(QString title)
{
    _title = title;
}

void CFolder::set_parent(CFolder * parent)
{
    _parent = parent;
}

bool CFolder::insert_to_db()
{
    QSqlQuery query(*_db->get_db());
    query.prepare(R"(
        INSERT INTO Folder (title, created_at, parent_id)
        VALUES (:title, :created_at, :parent_id)
    )");

    query.bindValue(":title", _title);
    query.bindValue(":created_at", QDateTime::currentSecsSinceEpoch());
    query.bindValue(":parent_id", (_parent != nullptr)? _parent->get_id(): QVariant(QVariant::Int));

    if (!query.exec()) {
        return false;
    }

    _id = query.lastInsertId().toInt();
    return true;
}

int CFolder::get_id() const
{
    return _id;
}
