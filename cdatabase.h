#ifndef CDATABASE_H
#define CDATABASE_H

#include <vector>
#include <QSqlDatabase>
#include <QString>
#include "cfolder.h"

class CNote;
class CFolder;

class CDataBase
{
public:
    CDataBase(const QString& path);
    ~CDataBase();

    bool is_open() const;

    bool update(CNote *note);
    bool insert(CNote * note);
    bool insert(CFolder * folder);

    bool update(CFolder *folder);
    bool removeNote(int noteId);
    bool removeFolder(int folderId);
    int countItemsInFolder(int folderId);

    std::vector<CFolder *> readAllFolders();
    std::vector<CNote *> readAllNotes();

    std::map<int,QString> findNotes(QString & pattern);

private:
    QSqlDatabase _db;
    QString _connectionName;
};

#endif // CDATABASE_H
