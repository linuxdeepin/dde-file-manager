#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
       return 0;
    }
    QApplication a(argc, argv);
    QString eventname = a.arguments().at(1);
    QString filepath = a.arguments().at(2);
    MainWindow w(eventname,filepath);
    w.show();
    w.move ((1333 - w.width())/2,(800 - w.height())/2);
    return a.exec();
}
