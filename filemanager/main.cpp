#include "app/global.h"
#include "app/logutil.h"
#include "app/filemanagerapp.h"
#include "app/singleapplication.h"

#include "widgets/commandlinemanager.h"

#include "../dialogs/dialogmanager.h"

#include "../shutil/mimesappsmanager.h"

#include <dthememanager.h>
#include <dwindow.h>

#include <QApplication>
#include <QDebug>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    LogUtil::registerLogger();

    SingleApplication app(argc, argv);

    app.setOrganizationName("deepin");
    app.setApplicationName(QObject::tr("dde-file-manager"));
    app.setApplicationVersion("v1.0");

    CommandLineManager::instance()->process();

    QString commandlineUrl = QDir::homePath();

    if (CommandLineManager::instance()->positionalArguments().count() > 0){
        commandlineUrl = CommandLineManager::instance()->positionalArguments().at(0);
    }

    MimesAppsManager::test();

    QString uniqueKey = "dde-file-manager";

    bool isSingleInstance  = app.setSingleInstance(uniqueKey);

    qDebug() << isSingleInstance << commandlineUrl;

    if (isSingleInstance){
        DThemeManager::instance()->setTheme("light");

        fileManagerApp->show(commandlineUrl);
        dialogManager;

        return app.exec();
    }else{
        SingleApplication::newClientProcess(uniqueKey);
    }
}
