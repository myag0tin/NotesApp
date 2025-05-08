#include "cdatabase.h"
#include "cnote.h"
#include "cfolder.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include <QVariant>
#include <QDebug>
#include <QMessageBox>
#include <qapplication.h>

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
    // QSqlDatabase::removeDatabase(_connectionName);
}

bool CDataBase::is_open() const
{
    return _db.isOpen();
}

bool CDataBase::update(CNote *note)
{
    if (!note || !_db.isOpen()) {
        return false;
    }

    QSqlQuery query(_db);
    query.prepare(R"(
        UPDATE Note
        SET title = :title,
            content = :content,
            parent_id = :parent_id
        WHERE id = :id
    )");

    query.bindValue(":title", note->get_title());
    query.bindValue(":content", note->get_content());
    query.bindValue(":parent_id", note->get_parent_id());
    query.bindValue(":id", note->get_id()); // Specify the note's ID to update the correct record

    if (!query.exec()) {
        qDebug() << "Ошибка обновления заметки:" << query.lastError().text();
        return false;
    }

    return true;
}

bool CDataBase::update(CFolder *folder)
{
    if (!folder || !_db.isOpen()) {
        return false;
    }

    QSqlQuery query(_db);
    query.prepare(R"(
        UPDATE Folder
        SET title = :title,
            parent_id = :parent_id
        WHERE id = :id
    )");

    query.bindValue(":title", folder->get_title());
    query.bindValue(":parent_id", folder->get_parent_id());
    query.bindValue(":id", folder->get_id());

    if (!query.exec()) {
        qDebug() << "Ошибка обновления папки:" << query.lastError().text();
        return false;
    }

    return true;
}

bool CDataBase::insert(CNote * note)
{
    if (!note || !_db.isOpen()) {
        return false;
    }

    QSqlQuery query(_db);
    query.prepare(R"(
        INSERT INTO Note (title, content, created_at, parent_id, deleted_id)
        VALUES (:title, :content, :created_at, :parent_id, :deleted_id)
    )");

    query.bindValue(":title", note->get_title());
    query.bindValue(":content", note->get_content());
    query.bindValue(":created_at", QDateTime::currentSecsSinceEpoch());
    //query.bindValue(":parent_id", (note->get_parent() != nullptr) ? note->get_parent()->get_id() : QVariant(QVariant::Int));
    query.bindValue(":parent_id", note->get_parent_id());
    query.bindValue(":deleted_id", 0);

    if (!query.exec()) {
        qDebug() << "Ошибка вставки заметки:" << query.lastError().text();
        return false;
    }

    // Update note's ID with the last inserted ID
    note->set_id(query.lastInsertId().toInt());
    return true;
}

bool CDataBase::insert(CFolder * folder)
{
    if (!folder || !_db.isOpen()) {
        return false;
    }

    QSqlQuery query(_db);
    query.prepare(R"(
        INSERT INTO Folder (title, created_at, parent_id, deleted_id)
        VALUES (:title, :created_at, :parent_id, :deleted_id)
    )");

    query.bindValue(":title", folder->get_title());
    query.bindValue(":created_at", QDateTime::currentSecsSinceEpoch());
    query.bindValue(":parent_id", folder->get_parent_id());
    query.bindValue(":deleted_id", 0);

    if (!query.exec()) {
        qDebug() << "Ошибка вставки папки:" << query.lastError().text();
        return false;
    }

    // Update folder's ID with the last inserted ID
    folder->set_id(query.lastInsertId().toInt());
    return true;
}

bool CDataBase::removeNote(int noteId)
{
    if (!_db.isOpen()) {
        return false;
    }

    QSqlQuery query(_db);
    query.prepare("DELETE FROM Note WHERE id = :id");
    query.bindValue(":id", noteId);

    if (!query.exec()) {
        qDebug() << "Ошибка удаления заметки:" << query.lastError().text();
        return false;
    }

    return true;
}

bool CDataBase::removeFolder(int folderId)
{
    if (!_db.isOpen()) {
        return false;
    }

    // Рекурсивно удаляем все заметки в папке
    QSqlQuery noteQuery(_db);
    noteQuery.prepare("SELECT id FROM Note WHERE parent_id = :folderId");
    noteQuery.bindValue(":folderId", folderId);
    if (!noteQuery.exec()) {
        qDebug() << "Ошибка получения заметок для удаления:" << noteQuery.lastError().text();
        return false;
    }

    while (noteQuery.next()) {
        int noteId = noteQuery.value("id").toInt();
        if (!removeNote(noteId)) {
            return false;
        }
    }

    // Рекурсивно удаляем все подпапки
    QSqlQuery folderQuery(_db);
    folderQuery.prepare("SELECT id FROM Folder WHERE parent_id = :folderId");
    folderQuery.bindValue(":folderId", folderId);
    if (!folderQuery.exec()) {
        qDebug() << "Ошибка получения подпапок для удаления:" << folderQuery.lastError().text();
        return false;
    }

    while (folderQuery.next()) {
        int subFolderId = folderQuery.value("id").toInt();
        if (!removeFolder(subFolderId)) {
            return false;
        }
    }

    // Удаляем саму папку
    QSqlQuery deleteQuery(_db);
    deleteQuery.prepare("DELETE FROM Folder WHERE id = :id");
    deleteQuery.bindValue(":id", folderId);
    if (!deleteQuery.exec()) {
        qDebug() << "Ошибка удаления папки:" << deleteQuery.lastError().text();
        return false;
    }

    return true;
}

int CDataBase::countItemsInFolder(int folderId)
{
    if (!_db.isOpen()) {
        return 0;
    }

    int count = 0;

    // Подсчет заметок
    QSqlQuery noteQuery(_db);
    noteQuery.prepare("SELECT COUNT(*) FROM Note WHERE parent_id = :folderId");
    noteQuery.bindValue(":folderId", folderId);
    if (noteQuery.exec() && noteQuery.next()) {
        count += noteQuery.value(0).toInt();
    } else {
        qDebug() << "Ошибка подсчета заметок:" << noteQuery.lastError().text();
    }

    // Подсчет подпапок
    QSqlQuery folderQuery(_db);
    folderQuery.prepare("SELECT id FROM Folder WHERE parent_id = :folderId");
    folderQuery.bindValue(":folderId", folderId);
    if (!folderQuery.exec()) {
        qDebug() << "Ошибка получения подпапок для подсчета:" << folderQuery.lastError().text();
        return count;
    }

    while (folderQuery.next()) {
        count++; // Учитываем саму подпапку
        int subFolderId = folderQuery.value("id").toInt();
        count += countItemsInFolder(subFolderId); // Рекурсивно подсчитываем содержимое подпапки
    }

    return count;
}

std::vector<CFolder *> CDataBase::readAllFolders()
{

    QSqlQuery query("SELECT * FROM Folder", _db);
    if (!query.exec()) {
        QString errorMsg = query.lastError().text();
        QMessageBox::critical(nullptr, "Database Error", "Query failed:\n" + errorMsg);
        QApplication::quit();
    }

    std::vector<CFolder *> result;
    while (query.next()) {
           result.push_back(new CFolder(query));
    }

    return result;
}

std::vector<CNote *> CDataBase::readAllNotes()
{

    QSqlQuery query("SELECT * FROM Note", _db);
    if (!query.exec()) {
        QString errorMsg = query.lastError().text();
        QMessageBox::critical(nullptr, "Database Error", "Query failed:\n" + errorMsg);
        QApplication::quit();
    }

    std::vector<CNote *> result;
    while (query.next()) {
        result.push_back(new CNote(query));
    }

    return result;
}




std::map<int,QString> CDataBase::findNotes(QString & pattern)
{
    std::map<int, QString> result;

    // Prepare the SQL query, using LIKE for pattern matching
    QSqlQuery query(_db);
    QString sql = "SELECT id, title FROM note WHERE content LIKE ?";

    if (!query.prepare(sql)) {
           qDebug() << "Query prepare failed:" << query.lastError();
           return result;
     }

     // Add wildcards to the pattern
     QString likePattern = "%" + pattern + "%";
     query.addBindValue(likePattern);

     // Execute and fetch results
     if (!query.exec()) {
           qDebug() << "Query execution failed:" << query.lastError();
           return result;
     }

      while (query.next()) {
           int id = query.value(0).toInt();
           QString title = query.value(1).toString();
           result[id] = title;
     }

     return result;

}
