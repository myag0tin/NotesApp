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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::foo()
{
    std::cout << "Hi!" << std::endl;
}


void MainWindow::on_testButton_clicked()
{
    QString path = "D:/__Programming/__DB/notes.db";
    CDataBase * db = new CDataBase(path);

    //QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    //db.setDatabaseName(path);

    //if (!db.open()) {
    //    qDebug() << "Ошибка подключения к БД:" << db.lastError().text();
     //   return;
    //}

    qDebug() << "База данных успешно открыта.";

    // ===== Добавление записи в Note =====
    CNote * myNote = new CNote(db);
    myNote->set_title("Новая заметка - класс");
    myNote->set_content("Содержимое новой заметки - класс");
    if (!myNote->insert_to_db()) {
        qDebug() << "Не удалось добавить заметку.";
    }
    delete myNote;

    // ===== Добавление записи в Folder через CFolder =====
    CFolder * folder = new CFolder(db);
    folder->set_title("Новая папка - класс");
    folder->set_parent(nullptr);

    if (!folder->insert_to_db()) {
        qDebug() << "Не удалось добавить папку.";
    } else {
        qDebug() << "Папка успешно добавлена с ID:" << folder->get_id();
    }
    delete folder;

    // Закрываем БД
    //db.close();
    //qDebug() << "База данных закрыта.";
}

