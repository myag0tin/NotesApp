#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QApplication>
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle("Toggle Mode");
    setModal(true);
    setFixedSize(290,70);

    setWindowIcon(QIcon("D:/__Programming/__NotesApp/NoteApp/settings_ico.png"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *label = new QLabel("Here you can change the theme of the NotesApp", this);
    layout->addWidget(label);

    switchModeButton = new QPushButton("Switch to Dark Mode", this);
    connect(switchModeButton, &QPushButton::clicked, this, &SettingsDialog::on_switchModeButton_clicked);
    layout->addWidget(switchModeButton);

    setLayout(layout);

    QSettings settings("NotesApp", "ThemeSettings");
    darkModeEnabled = settings.value("darkMode", false).toBool();
    applyTheme(darkModeEnabled);
}

SettingsDialog::~SettingsDialog()
{
    delete switchModeButton;
}

void SettingsDialog::applyTheme(bool darkMode)
{
    if (darkMode) {
        qApp->setStyleSheet(R"(
            QWidget {
                background-color: #333333;
                color: #ffffff;
            }
            QPushButton {
                background-color: #1e1e1e;
                color: #ffffff;
                border: 4px solid #000;
                border-radius: 0px;
                padding: 0px;
            }
            QPushButton:hover {
                background-color: #2c2c2c;
            }
            QLineEdit, QTextEdit {
                background-color: #1e1e1e;
                color: #ffffff;
                border: 0px solid #333;
            }
        )");
        switchModeButton->setText("Switch to Light Mode");
    } else {
        qApp->setStyleSheet("");
        switchModeButton->setText("Switch to Dark Mode");
    }
}

void SettingsDialog::on_switchModeButton_clicked()
{
    darkModeEnabled = !darkModeEnabled;
    applyTheme(darkModeEnabled);

    QSettings settings("NotesApp", "ThemeSettings");
    settings.setValue("darkMode", darkModeEnabled);
}
