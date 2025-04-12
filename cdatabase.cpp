#include "cdatabase.h"
#include <QSqlError>
#include <QDebug>

CDataBase::CDataBase(const QString& path)
{
    // Создаём уникальное имя подключения
    _connectionName = "notes_connection";

    if (QSqlDatabase::contains(_connectionName)) {
        _db = QSqlDatabase::database(_connectionName);
    } else {
        _db = QSqlDatabase::addDatabase("QSQLITE", _connectionName);
        _db.setDatabaseName(path);
    }

    if (!_db.open()) {
        qDebug() << "Ошибка подключения к БД:" << _db.lastError().text();
    }
}

CDataBase::~CDataBase()
{
    if (_db.isOpen()) {
        _db.close();
        qDebug() << "База данных закрыта.";
    }

    // Удаляем подключение
    QSqlDatabase::removeDatabase(_connectionName);
}

bool CDataBase::is_open() const
{
    return _db.isOpen();
}

QSqlDatabase* CDataBase::get_db()
{
    return &_db;
}
