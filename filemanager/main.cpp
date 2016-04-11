#include <QApplication>
#include <QDebug>
#include <QUrl>
#include <QLabel>
#include <dthememanager.h>
#include <dwindow.h>
#include <QLocalSocket>
#include "app/global.h"
#include "../app/singleapplication.h"
#include "app/logutil.h"
#include "widgets/commandlinemanager.h"
#include <QJsonObject>


DWIDGET_USE_NAMESPACE


int main(int argc, char *argv[])
{
    LogUtil::registerLogger();
    QString uniqueKey = "dde-file-manager";
    SingleApplication app(argc, argv);

    app.setOrganizationName("deepin");
    app.setApplicationName(QObject::tr("dde-file-manager"));
    app.setApplicationVersion("v1.0");

    CommandLineManager::instance()->process();
    bool isSingleInstance  = app.setSingleInstance(uniqueKey);
    QString commandlineUrl;
    if (CommandLineManager::instance()->positionalArguments().count() > 0){
        commandlineUrl = CommandLineManager::instance()->positionalArguments().at(0);
    }
    qDebug() << isSingleInstance << commandlineUrl;
    if (isSingleInstance){
        DThemeManager::instance()->setTheme("light");
        fileManagerApp->show();
        return app.exec();
    }else{
        SingleApplication::newClientProcess(uniqueKey);
    }
}
