#include "csearchdialog.h"
#include "ui_csearchdialog.h"
#include "cdatabase.h"
#include <iostream>

CSearchDialog::CSearchDialog(CDataBase * db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSearchDialog),
    _db(db)
{
    ui->setupUi(this);
    setWindowIcon(QIcon("D:/__Programming/Icons/search_bar_icon.png"));
    setWindowTitle("Item Search");
    ui->editLookupString->setPlaceholderText("Search here...");
    setFixedSize(337, 258);

    addPlaceholderIfEmpty();
    bool connected = connect(ui->listItems, &QListWidget::itemDoubleClicked, this, &CSearchDialog::on_listItems_doubleClicked);
    qDebug() << "Double-click signal connected:" << connected;

}

CSearchDialog::~CSearchDialog()
{
    delete ui;
}

void CSearchDialog::on_btnSearch_clicked()
{
    QString txt = ui->editLookupString->text();

    fillItemList(_db->findNotes(txt));

}

void CSearchDialog::fillItemList(std::map<int, QString> items)
{
    // Очищаем старые результаты
    ui->listItems->clear();

    if (items.empty()) {
        // Если результатов нет, показываем заглушку
        QListWidgetItem *placeholderItem = new QListWidgetItem(tr("Matching notes will appear here..."));
        placeholderItem->setTextAlignment(Qt::AlignCenter);
        placeholderItem->setForeground(QBrush(Qt::gray));
        placeholderItem->setFlags(Qt::NoItemFlags); // Неинтерактивная заглушка
        ui->listItems->addItem(placeholderItem);
    } else {
        // Заполняем список результатами поиска
        for (const auto &pair : items) {
            QString itemText = QString("%1").arg(pair.second);
            QListWidgetItem *item = new QListWidgetItem(itemText);
            item->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
            item->setData(Qt::UserRole, pair.first); // Сохраняем ID заметки
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled); // Убедимся, что элемент интерактивен
            ui->listItems->addItem(item);
            qDebug() << "Added item: ID =" << pair.first << ", Title =" << itemText;
        }
    }
}

void CSearchDialog::addPlaceholderIfEmpty()
{
    // Если список пуст, добавляем заглушку
    if (ui->listItems->count() == 0) {
        QListWidgetItem *placeholderItem = new QListWidgetItem(tr("Matching notes will appear here..."));
        placeholderItem->setTextAlignment(Qt::AlignCenter);
        placeholderItem->setForeground(QBrush(Qt::gray));
        placeholderItem->setFlags(Qt::NoItemFlags);
        ui->listItems->addItem(placeholderItem);
    }
}

void CSearchDialog::on_listItems_doubleClicked(QListWidgetItem *item)
{
    qDebug() << "Double-click detected on item:" << (item ? item->text() : "null");
    // Проверяем, валиден ли элемент и есть ли ID
    if (item && item->data(Qt::UserRole).isValid()) {
        int noteId = item->data(Qt::UserRole).toInt();
        qDebug() << "Emitting noteSelected with noteId:" << noteId;
        emit noteSelected(noteId); // Эмитируем сигнал с ID заметки
        accept(); // Закрываем диалог
    } else {
        qDebug() << "Invalid item or no note ID";
    }
}
