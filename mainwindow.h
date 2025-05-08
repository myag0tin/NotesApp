#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "cdatabase.h"
#include "cfolder.h"
#include "cnote.h"
#include <QTreeWidgetItem>
#include "settingsdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:

    void onTreeItemClicked(QTreeWidgetItem* item, int column);
    void onTreeWidgetItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void onNoteContentChanged();

    void showContextMenu(const QPoint &pos);

    void on_actionCreateNote_triggered();
    void on_actionCreateFolder_triggered();
    void on_actionToggleUiMode_triggered();
    void on_actionSearch_triggered();
    void on_actionHelp_triggered();
    void on_noteSelected(int noteId);

private:
    Ui::MainWindow *ui;

    CDataBase * _db;

    void initDatabes(QString path);
    void intitializeStructureWidget();

    QTreeWidget *_structureWidget;

    bool darkModeEnabled = false;

    QPushButton *settingsButton;

    QTimer * _saveTimeout;
    QTreeWidgetItem* findFolderItem(QTreeWidgetItem *item, int folderId);
    QTreeWidgetItem* findNoteItem(QTreeWidgetItem *item, int noteId);

};
#endif // MAINWINDOW_H
