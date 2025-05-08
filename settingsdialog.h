#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QPushButton>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();


private slots:
    void on_switchModeButton_clicked();

private:
    QPushButton *switchModeButton;
    bool darkModeEnabled;

    void applyTheme(bool darkMode);

};

#endif // SETTINGSDIALOG_H
