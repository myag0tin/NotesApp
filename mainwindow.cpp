#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

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

    // 1. Открываем базу данных
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);

    if (!db.open()) {
        qDebug() << "Ошибка подключения к БД:" << db.lastError().text();
        return;
    }

    qDebug() << "База данных успешно открыта.";

    QSqlQuery query;

    // Получаем текущее время как timestamp (int) и как текст (QString)
    qint64 timestamp = QDateTime::currentSecsSinceEpoch();
    QString createdAtStr = QDateTime::currentDateTime().toString(Qt::ISODate);

    // ===== Добавление записи в Note =====
    QString noteTitle = "Новая заметка";
    QString noteContent = "Содержимое новой заметки";
    int parentNoteId = 0; // или другой ID, если есть иерархия

    query.prepare(R"(
        INSERT INTO Note (title, content, created_at, parent_id)
        VALUES (:title, :content, :created_at, :parent_id)
    )");
    query.bindValue(":title", noteTitle);
    query.bindValue(":content", noteContent);
    query.bindValue(":created_at", timestamp); // int
    query.bindValue(":parent_id", parentNoteId);

    if (!query.exec()) {
        qDebug() << "Ошибка вставки в Note:" << query.lastError().text();
    } else {
        qDebug() << "Запись в Note успешно добавлена.";
    }

    // ===== Добавление записи в Folder =====
    QString folderTitle = "Новая папка";
    int parentFolderId = 0;

    query.prepare(R"(
        INSERT INTO Folder (title, created_at, parent_id)
        VALUES (:title, :created_at, :parent_id)
    )");
    query.bindValue(":title", folderTitle);
    query.bindValue(":created_at", timestamp); // unix время в виде int
    query.bindValue(":parent_id", parentFolderId);

    if (!query.exec()) {
        qDebug() << "Ошибка вставки в Folder:" << query.lastError().text();
    } else {
        qDebug() << "Запись в Folder успешно добавлена.";
    }

    // Закрываем БД
    db.close();
    qDebug() << "База данных закрыта.";
}
