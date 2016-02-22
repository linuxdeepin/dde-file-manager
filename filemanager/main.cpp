#include <QApplication>
#include <QDebug>
#include <QUrl>
#include "app/filemanagerapp.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    FileManagerApp fileManagerApp;
    fileManagerApp.show();

    int reslut = app.exec();

    qDebug() << (QUrl("/home") == QUrl("file:///home"));
    qDebug() << "exits " << app.applicationName() << reslut;
}
