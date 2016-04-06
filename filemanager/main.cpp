#include <QApplication>
#include <QDebug>
#include <QUrl>
#include <QLabel>
#include <dthememanager.h>
#include <dwindow.h>
#include "app/global.h"

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DThemeManager::instance()->setTheme("light");
    fileManagerApp->show();

    return app.exec();
}
