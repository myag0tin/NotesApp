#include "cnote.h"
#include <QDateTime>
#include <QSqlQuery>
#include <QVariant>

CNote::CNote(CDataBase * db) : _db(db), _parent(nullptr), _id(-1)
{
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

bool CNote::insert_to_db()
{
    QSqlQuery query(*_db->get_db());
    query.prepare(R"(
        INSERT INTO Note (title, content, created_at, parent_id)
        VALUES (:title, :content, :created_at, :parent_id)
    )");

    query.bindValue(":title", _title);
    query.bindValue(":content", _content);
    query.bindValue(":created_at", QDateTime::currentSecsSinceEpoch());
    query.bindValue(":parent_id", (_parent != nullptr)? _parent->get_id(): QVariant(QVariant::Int));

    if (!query.exec()) {
        return false;
    }

    _id = query.lastInsertId().toInt();
    return true;
}

int CNote::get_id() const
{
    return _id;
}
