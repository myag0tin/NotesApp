#include "mainwindow.h"
#include <QApplication>
#include <QMetaType>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<CNote*>("CNote*");
    qRegisterMetaType<CFolder*>("CFolder*");

    MainWindow w;
    w.setWindowTitle("NotesApp");
    w.show();
    return a.exec();
}
