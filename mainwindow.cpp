#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include "cnote.h"
#include "cfolder.h"
#include "cdatabase.h"
#include "settingsdialog.h"
#include <QMessageBox>
#include <QTimer>
#include <QMenu>
#include <QInputDialog>
#include <QComboBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include "csearchdialog.h"
#include <QLabel>
#include <QTextBrowser>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _structureWidget = nullptr;

    setFixedSize(size());

    initDatabes("D:/__Programming/__DB/notes.db");
    intitializeStructureWidget();

    _saveTimeout = new QTimer(this);
    _saveTimeout->setInterval(300);
    _saveTimeout->start();

    ui->structureWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->structureWidget, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
    connect(ui->structureWidget, &QTreeWidget::itemClicked, this, &MainWindow::onTreeItemClicked);
    connect(ui->structureWidget, &QTreeWidget::currentItemChanged, this, &MainWindow::onTreeWidgetItemChanged);
    connect(_saveTimeout, &QTimer::timeout, this, &MainWindow::onNoteContentChanged);


    QAction *searchAction = new QAction("Search", this);
    menuBar()->addAction(searchAction);
    connect(searchAction, &QAction::triggered, this, &MainWindow::on_actionSearch_triggered);

}

MainWindow::~MainWindow()
{
    auto folders = _db->readAllFolders();
    for (auto* folder : folders) {
        delete folder;
    }
    auto notes = _db->readAllNotes();
    for (auto* note : notes) {
        delete note;
    }
    delete _db;
    delete ui;
}

void MainWindow::initDatabes(QString path)
{
    _db = new CDataBase(path);
}

void MainWindow::intitializeStructureWidget()
{
    _structureWidget = ui->structureWidget;
    _structureWidget->setHeaderLabels(QStringList() << "Vault");

    _structureWidget->header()->setStyleSheet("QHeaderView::section { color: black; }");

    std::map<int, QTreeWidgetItem*> tfMap;
    auto folders = _db->readAllFolders();
    qDebug() << "Reading" << folders.size() << "folders from database";
    for (auto it = folders.begin(); it != folders.end(); ++it) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, (*it)->get_title());
        item->setIcon(0, style()->standardIcon(QStyle::SP_DirOpenIcon));
        item->setData(0, Qt::UserRole + 1, QVariant::fromValue(*it));
        qDebug() << "Saving CFolder* for folder ID:" << (*it)->get_id()
                 << "Title:" << (*it)->get_title()
                 << "Data valid:" << item->data(0, Qt::UserRole + 1).isValid()
                 << "Type:" << item->data(0, Qt::UserRole + 1).typeName();
        tfMap[(*it)->get_id()] = item;
    }

    for (auto it = folders.begin(); it != folders.end(); ++it) {
        CFolder *folder = *it;
        QTreeWidgetItem *folderItem = tfMap[folder->get_id()];
        int parent_id = folder->get_parent_id();
        qDebug() << "Processing folder ID:" << folder->get_id() << "Parent ID:" << parent_id;
        if (parent_id > 0 && tfMap.find(parent_id) != tfMap.end()) {
            QTreeWidgetItem *parentItem = tfMap[parent_id];
            parentItem->addChild(folderItem);
        } else {
            _structureWidget->addTopLevelItem(folderItem);
        }
    }

    auto notes = _db->readAllNotes();
    qDebug() << "Reading" << notes.size() << "notes from database";
    for (auto it = notes.begin(); it != notes.end(); ++it) {
        CNote *note = *it;
        QTreeWidgetItem *noteItem = new QTreeWidgetItem();
        noteItem->setData(0, Qt::UserRole, QVariant::fromValue(note));
        noteItem->setText(0, note->get_title());
        noteItem->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));

        int parent_id = note->get_parent_id();
        if (tfMap.find(parent_id) != tfMap.end()) {
            QTreeWidgetItem *parentItem = tfMap[parent_id];
            parentItem->addChild(noteItem);
        } else {
            ui->structureWidget->addTopLevelItem(noteItem);
        }
    }
}


void MainWindow::onTreeItemClicked(QTreeWidgetItem* item, int column)
{
    QVariant data = item->data(0, Qt::UserRole);
    CNote* note = data.value<CNote*>();
    if (note) {
        ui->noteDescription->setPlainText(note->get_content());
        ui->noteDescription->setEnabled(true);
    }
}

void MainWindow::onTreeWidgetItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (current != previous)
    {
        ui->noteDescription->setEnabled(false);
        ui->noteDescription->setPlainText("");
    }
}

void MainWindow::onNoteContentChanged()
{
    QTreeWidgetItem *item = ui->structureWidget->currentItem();
    if (item != nullptr)
    {
        QVariant data = item->data(0, Qt::UserRole);
        CNote* note = data.value<CNote*>();
        QString newContent = ui->noteDescription->toPlainText();
        if (note != nullptr && ui->noteDescription->toPlainText().length() && (note->get_content() != newContent))
        {
            note->set_content(newContent);
            _db->update(note);
        }
    }
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = ui->structureWidget->itemAt(pos);
    qDebug() << "Clicked position:" << pos << "Item found:" << (item != nullptr);

    if (!item) {
        qDebug() << "No item clicked (empty area)";
        return;
    }

    QMenu contextMenu(tr("Контекстное меню"), this);

    QVariant data = item->data(0, Qt::UserRole);
    CNote *note = data.value<CNote*>();
    if (note && data.isValid()) {
        qDebug() << "Item is a note. Title:" << note->get_title() << "ID:" << note->get_id();
        QAction *actionRename = new QAction("Rename", &contextMenu);
        QAction *actionMoveToFolder = new QAction("Move to Folder", &contextMenu);
        QAction *actionDelete = new QAction("Delete", &contextMenu);

        connect(actionRename, &QAction::triggered, this, [this, item, note]() {
                    QDialog dialog(this);
                    dialog.setWindowTitle(tr("Rename a Note"));
                    QVBoxLayout layout(&dialog);

                    QLabel *label = new QLabel(tr("Note name:"));
                                layout.addWidget(label);

                    QLineEdit *lineEdit = new QLineEdit(note->get_title());
                    layout.addWidget(lineEdit);

                    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
                    layout.addWidget(&buttonBox);

                    dialog.adjustSize();
                    dialog.setFixedSize(dialog.size());

                    if (dialog.exec() == QDialog::Accepted) {
                        QString newTitle = lineEdit->text();
                        if (!newTitle.isEmpty()) {
                            note->set_title(newTitle);
                            if (_db->update(note)) {
                                item->setText(0, newTitle);
                                qDebug() << "Заметка переименована:" << newTitle;
                            } else {
                                QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось переименовать заметку."));
                            }
                        }
                    }
                });

        connect(actionMoveToFolder, &QAction::triggered, this, [this, item, note]() {
            QDialog dialog(this);
            dialog.setWindowTitle(tr("Move a Note to ..."));
            QVBoxLayout layout(&dialog);

            QComboBox comboBox;
            comboBox.addItem(tr("No Folder"), 0);
            auto folders = _db->readAllFolders();
            for (const auto* folder : folders) {
                if (folder->get_id() != note->get_parent_id()) {
                    comboBox.addItem(folder->get_title(), folder->get_id());
                }
            }

            layout.addWidget(&comboBox);
            QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
            connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
            layout.addWidget(&buttonBox);

            dialog.adjustSize();
            dialog.setFixedSize(dialog.size());

            if (dialog.exec() == QDialog::Accepted) {
                int newParentId = comboBox.currentData().toInt();
                note->set_parent_id(newParentId);
                if (_db->update(note)) {
                    QTreeWidgetItem *parentItem = nullptr;
                    if (newParentId > 0) {
                        for (int i = 0; i < ui->structureWidget->topLevelItemCount(); ++i) {
                            QTreeWidgetItem *topItem = ui->structureWidget->topLevelItem(i);
                            QVariant folderData = topItem->data(0, Qt::UserRole + 1);
                            CFolder *folder = folderData.value<CFolder*>();
                            if (folder && folder->get_id() == newParentId) {
                                parentItem = topItem;
                                break;
                            }
                            parentItem = findFolderItem(topItem, newParentId);
                            if (parentItem) break;
                        }
                    }
                    if (item->parent()) {
                        item->parent()->removeChild(item);
                    } else {
                        int index = ui->structureWidget->indexOfTopLevelItem(item);
                        ui->structureWidget->takeTopLevelItem(index);
                    }
                    if (parentItem) {
                        parentItem->addChild(item);
                    } else {
                        ui->structureWidget->addTopLevelItem(item);
                    }
                    qDebug() << "Заметка перемещена в папку ID:" << newParentId;
                } else {
                    QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось переместить заметку."));
                }
            }
            for (auto* folder : folders) {
                delete folder;
            }
        });

        connect(actionDelete, &QAction::triggered, this, [this, item, note]() {
            if (_db->removeNote(note->get_id())) {
                if (item->parent()) {
                    item->parent()->removeChild(item);
                } else {
                    int index = ui->structureWidget->indexOfTopLevelItem(item);
                    ui->structureWidget->takeTopLevelItem(index);
                }
                delete item;
                qDebug() << "Заметка удалена:" << note->get_title();
            } else {
                QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось удалить заметку."));
            }
        });

        contextMenu.addAction(actionRename);
        contextMenu.addAction(actionMoveToFolder);
        contextMenu.addAction(actionDelete);
        qDebug() << "Added 'Переименовать', 'Переместить в папку', 'Удалить' actions for note";
    } else {
        QVariant folderData = item->data(0, Qt::UserRole + 1);
        qDebug() << "Folder data valid:" << folderData.isValid()
                 << "Type:" << folderData.typeName()
                 << "Item text:" << item->text(0);
        CFolder *folder = folderData.value<CFolder*>();
        if (!folder || !folderData.isValid()) {
            qDebug() << "Ошибка: Не удалось получить CFolder* для элемента. Item text:" << item->text(0);
            QMessageBox::critical(this, tr("Ошибка"), tr("Не удалось определить папку."));
            return;
        }
        qDebug() << "Item is a folder. Title:" << folder->get_title() << "ID:" << folder->get_id();

        QAction *actionRename = new QAction("Rename", &contextMenu);
        QAction *actionAddNote = new QAction("Add Note", &contextMenu);
        QAction *actionMoveToFolder = new QAction("Move to Folder", &contextMenu);
        QAction *actionDelete = new QAction("Delete", &contextMenu);

        connect(actionRename, &QAction::triggered, this, [this, item, folder]() {
                    QDialog dialog(this);
                    dialog.setWindowTitle(tr("Rename a Folder"));
                    QVBoxLayout layout(&dialog);

                    QLabel *label = new QLabel(tr("Folder name:"));
                                layout.addWidget(label);

                    QLineEdit *lineEdit = new QLineEdit(folder->get_title());
                    layout.addWidget(lineEdit);

                    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
                    layout.addWidget(&buttonBox);

                    dialog.adjustSize();
                    dialog.setFixedSize(dialog.size());

                    if (dialog.exec() == QDialog::Accepted) {
                        QString newTitle = lineEdit->text();
                        if (!newTitle.isEmpty()) {
                            folder->set_title(newTitle);
                            if (_db->update(folder)) {
                                item->setText(0, newTitle);
                                qDebug() << "Папка переименована:" << newTitle;
                            } else {
                                QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось переименовать папку."));
                            }
                        }
                    }
                });

        connect(actionAddNote, &QAction::triggered, this, [this, item, folder]() {
                    QDialog dialog(this);
                    dialog.setWindowTitle(tr("Create a Note"));
                    QVBoxLayout layout(&dialog);

                    QLabel *label = new QLabel(tr("Note name:"));
                    layout.addWidget(label);

                    QLineEdit *lineEdit = new QLineEdit();
                    layout.addWidget(lineEdit);

                    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
                    layout.addWidget(&buttonBox);

                    dialog.adjustSize();
                    dialog.setFixedSize(dialog.size());

                    if (dialog.exec() == QDialog::Accepted) {
                        QString title = lineEdit->text();
                        if (!title.isEmpty()) {
                            CNote *note = new CNote();
                            note->set_title(title);
                            note->set_content("");
                            note->set_parent_id(folder->get_id());
                            if (_db->insert(note)) {
                                QTreeWidgetItem *noteItem = new QTreeWidgetItem();
                                noteItem->setData(0, Qt::UserRole, QVariant::fromValue(note));
                                noteItem->setText(0, note->get_title());
                                noteItem->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
                                item->addChild(noteItem);
                                qDebug() << "Note created in folder ID:" << folder->get_id() << " Title:" << note->get_title();
                            } else {
                                delete note;
                                QMessageBox::warning(this, tr("Error"), tr("Failed to create Note"));
                            }
                        }
                    }
                });

        connect(actionMoveToFolder, &QAction::triggered, this, [this, item, folder]() {
            QDialog dialog(this);
            dialog.setWindowTitle(tr("Move a Folder to ..."));
            QVBoxLayout layout(&dialog);

            QComboBox comboBox;
            comboBox.addItem(tr("No Folder"), 0);
            auto folders = _db->readAllFolders();
            for (const auto* f : folders) {
                if (f->get_id() != folder->get_id() && f->get_id() != folder->get_parent_id()) {
                    comboBox.addItem(f->get_title(), f->get_id());
                }
            }

            layout.addWidget(&comboBox);
            QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
            connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
            layout.addWidget(&buttonBox);

            dialog.adjustSize();
            dialog.setFixedSize(dialog.size());

            if (dialog.exec() == QDialog::Accepted) {
                int newParentId = comboBox.currentData().toInt();
                folder->set_parent_id(newParentId);
                if (_db->update(folder)) {
                    QTreeWidgetItem *parentItem = nullptr;
                    if (newParentId > 0) {
                        for (int i = 0; i < ui->structureWidget->topLevelItemCount(); ++i) {
                            QTreeWidgetItem *topItem = ui->structureWidget->topLevelItem(i);
                            QVariant folderData = topItem->data(0, Qt::UserRole + 1);
                            CFolder *f = folderData.value<CFolder*>();
                            if (f && f->get_id() == newParentId) {
                                parentItem = topItem;
                                break;
                            }
                            parentItem = findFolderItem(topItem, newParentId);
                            if (parentItem) break;
                        }
                    }
                    if (item->parent()) {
                        item->parent()->removeChild(item);
                    } else {
                        int index = ui->structureWidget->indexOfTopLevelItem(item);
                        ui->structureWidget->takeTopLevelItem(index);
                    }
                    if (parentItem) {
                        parentItem->addChild(item);
                    } else {
                        ui->structureWidget->addTopLevelItem(item);
                    }
                    qDebug() << "Папка перемещена в папку ID:" << newParentId;
                } else {
                    QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось переместить папку."));
                }
            }
            for (auto* f : folders) {
                delete f;
            }
        });

        connect(actionDelete, &QAction::triggered, this, [this, item, folder]() {
            int itemCount = _db->countItemsInFolder(folder->get_id());
            if (itemCount > 0) {
                QString message = tr("This will delete the folder and %1 item(s) inside it. Continue?").arg(itemCount);

                QMessageBox msgBox(this);
                msgBox.setWindowTitle(tr("Delete Confirmation"));
                msgBox.setText(message);
                msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Cancel);
                msgBox.setWindowIcon(QIcon("D:/__Programming/Icons/alert.ico"));
                msgBox.setIcon(QMessageBox::Question);

                if (msgBox.exec() != QMessageBox::Ok) {
                    return;
                }
            }

            if (_db->removeFolder(folder->get_id())) {
                if (item->parent()) {
                    item->parent()->removeChild(item);
                } else {
                    int index = ui->structureWidget->indexOfTopLevelItem(item);
                    ui->structureWidget->takeTopLevelItem(index);
                }
                delete item;
                qDebug() << "Папка удалена:" << folder->get_title();
            } else {
                QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось удалить папку."));
            }
        });

        contextMenu.addAction(actionRename);
        contextMenu.addAction(actionAddNote);
        contextMenu.addAction(actionMoveToFolder);
        contextMenu.addAction(actionDelete);
        qDebug() << "Added 'Переименовать', 'Добавить заметку', 'Переместить в папку', 'Удалить' actions for folder";
    }

    qDebug() << "Total actions in menu:" << contextMenu.actions().count();
    qDebug() << "Showing menu at global pos:" << ui->structureWidget->mapToGlobal(pos);
    contextMenu.exec(ui->structureWidget->mapToGlobal(pos));
}

QTreeWidgetItem* MainWindow::findFolderItem(QTreeWidgetItem *item, int folderId)
{
    QVariant folderData = item->data(0, Qt::UserRole + 1);
    CFolder *folder = folderData.value<CFolder*>();
    if (folder && folder->get_id() == folderId) {
        return item;
    }
    for (int i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem *child = item->child(i);
        QTreeWidgetItem *result = findFolderItem(child, folderId);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

void MainWindow::on_actionCreateNote_triggered() {
    bool ok;
    QString title = QInputDialog::getText(this, tr("Create a Note"),
                                         tr("Note name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !title.isEmpty()) {
        CNote *note = new CNote();
        note->set_title(title);
        note->set_content("");
        note->set_parent_id(0);
        if (_db->insert(note)) {
            QTreeWidgetItem *noteItem = new QTreeWidgetItem();
            noteItem->setData(0, Qt::UserRole, QVariant::fromValue(note));
            noteItem->setText(0, note->get_title());
            noteItem->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
            ui->structureWidget->addTopLevelItem(noteItem);
            qDebug() << "Note created: ID" << note->get_id() << "Title:" << note->get_title();
        } else {
            delete note;
            QMessageBox::warning(this, tr("Error"), tr("Failed to create Note"));
        }
    }
}


void MainWindow::on_actionCreateFolder_triggered()
{
    bool ok;
    QString title = QInputDialog::getText(this, tr("Create a Folder"),
                                         tr("Folder name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !title.isEmpty()) {
        CFolder *folder = new CFolder();
        folder->set_title(title);
        folder->set_parent_id(0);
        folder->set_created_at(QDateTime::currentSecsSinceEpoch());
        folder->set_deleted_id(0);
        if (_db->insert(folder)) {
            QTreeWidgetItem *folderItem = new QTreeWidgetItem();
            folderItem->setData(0, Qt::UserRole + 1, QVariant::fromValue(folder));
            folderItem->setText(0, folder->get_title());
            folderItem->setIcon(0, style()->standardIcon(QStyle::SP_DirOpenIcon));
            ui->structureWidget->addTopLevelItem(folderItem);
            qDebug() << "Folder created: ID" << folder->get_id() << "Title:" << folder->get_title();
        } else {
            delete folder;
            QMessageBox::warning(this, tr("Error"), tr("Failed to create Folder"));
        }
    }
}


void MainWindow::on_actionToggleUiMode_triggered()
{
    SettingsDialog *settings = new SettingsDialog(this);
    settings->setWindowModality(Qt::ApplicationModal);
    settings->show();
}

void MainWindow::on_actionSearch_triggered()
{
    CSearchDialog dlg(_db, this);
    bool connected = connect(&dlg, &CSearchDialog::noteSelected, this, &MainWindow::on_noteSelected);
    qDebug() << "noteSelected signal connected:" << connected;
    dlg.exec();
}

void MainWindow::on_noteSelected(int noteId)
{
    qDebug() << "Received noteSelected with noteId:" << noteId;
    QTreeWidgetItem *noteItem = nullptr;
    for (int i = 0; i < _structureWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *topItem = _structureWidget->topLevelItem(i);
        noteItem = findNoteItem(topItem, noteId);
        if (noteItem) break;
    }

    if (noteItem) {
        qDebug() << "Found note item with title:" << noteItem->text(0);
        _structureWidget->setCurrentItem(noteItem);
        onTreeItemClicked(noteItem, 0);
    } else {
        qDebug() << "Note with ID" << noteId << "not found in tree";
        QMessageBox::warning(this, tr("Ошибка"), tr("Заметка не найдена."));
    }
}

QTreeWidgetItem* MainWindow::findNoteItem(QTreeWidgetItem *item, int noteId)
{
    QVariant noteData = item->data(0, Qt::UserRole);
    CNote *note = noteData.value<CNote*>();
    if (note && note->get_id() == noteId) {
        qDebug() << "Note found: ID =" << noteId << ", Title =" << item->text(0);
        return item;
    }
    for (int i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem *child = item->child(i);
        QTreeWidgetItem *result = findNoteItem(child, noteId);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

void MainWindow::on_actionHelp_triggered()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Help");

    msgBox.setText(
            "<h1>NotesApp</h1>"
            "<p>Welcome to <b>NotesApp</b>! This application is designed to help you create and organize your notes.</p>"
            "<h2>Main Features:</h2>"
            "<ul>"
            "<li><b>Create a Note (Folder)</b>: Toolbar → File → Create Note (Create Folder) or you can right-click on a created Folder → Add Note</li>"
            "<li><b>Edit</b>: Right-click in Vault on a created Note (Folder), thereafter you can Rename, Move to Folder, Delete or Add Note (only for Folder)</li>"
            "<li><b>Switch App Theme</b>: Toolbar → Settings → Toggle Mode → Switch to Dark Mode</li>"
            "<li><b>Item Search</b>: Toolbar → Search → Enter a key symbol (word or phrase) to search for relevant notes</li>"
            "</ul>"
            "<div style='text-align: right; margin: 0; padding: 0;'><p style='margin: 0; padding: 0;'>NotesApp 2025</p></div>"
            "<div style='text-align: right; margin: 0; padding: 0;'><p style='margin: 0; padding: 0;'><font color='green'>Aleksei Miagotin</font></p></div>"


        );

    msgBox.setWindowIcon(QIcon("D:/__Programming/__NotesApp/NoteApp/settings_ico.png"));
    msgBox.exec();
}
