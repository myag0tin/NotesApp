#ifndef CSEARCHDIALOG_H
#define CSEARCHDIALOG_H

#include <QDialog>
#include <QListWidget>

class CDataBase;

namespace Ui {
class CSearchDialog;
}

class CSearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CSearchDialog(CDataBase * db, QWidget *parent = nullptr);
    ~CSearchDialog();

signals:
    void noteSelected(int noteId);

private slots:
    void on_btnSearch_clicked();
    void on_listItems_doubleClicked(QListWidgetItem *item);

private:
    Ui::CSearchDialog *ui;

    CDataBase * _db;
    void fillItemList(std::map<int, QString> items);
    void addPlaceholderIfEmpty();


};

#endif // CSEARCHDIALOG_H
