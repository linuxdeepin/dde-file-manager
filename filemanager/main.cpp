#include <QApplication>
#include <QDebug>
#include <QUrl>

#include <dthememanager.h>

#include "app/filemanagerapp.h"

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DThemeManager::instance()->setTheme("light");

    FileManagerApp *fileManagerApp = FileManagerApp::instance();
    fileManagerApp->show();

    int reslut = app.exec();

    qDebug() << (QUrl("/home") == QUrl("file:///home"));
    qDebug() << "exits " << app.applicationName() << reslut;
}
