#include "durl.h"

#include "app/global.h"
#include "app/logutil.h"
#include "app/filemanagerapp.h"
#include "app/singleapplication.h"

#include "widgets/commandlinemanager.h"

#include "../dialogs/dialogmanager.h"

#include "../shutil/mimesappsmanager.h"
#include "../dialogs/openwithdialog.h"

#include <dthememanager.h>
#include <dwindow.h>

#include <QApplication>
#include <QDebug>
#include <QThreadPool>
#include <QTranslator>
#include <QImageReader>

#include "../controllers/pathmanager.h"

#ifdef ENABLE_PPROF
#include <gperftools/profiler.h>
#endif

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
#ifdef ENABLE_PPROF
    ProfilerStart("pprof.prof");
#endif
    SingleApplication app(argc, argv);

    app.setOrganizationName("deepin");
    app.setApplicationName(QObject::tr("dde-file-manager"));
    app.setApplicationVersion("v1.0");

    LogUtil::registerLogger();

    QTranslator translator;
    translator.load(APPSHAREDIR"/translations/dde-file-manager_" + QLocale::system().name());
    app.installTranslator(&translator);

    CommandLineManager::instance()->process();

    DUrl commandlineUrl;

    if (CommandLineManager::instance()->positionalArguments().count() > 0){
        commandlineUrl = DUrl::fromUserInput(CommandLineManager::instance()->positionalArguments().at(0));
        qDebug() << commandlineUrl;
    } else {
        commandlineUrl = DUrl::fromLocalFile(QDir::homePath());
        qDebug() << commandlineUrl;
    }

    QThreadPool::globalInstance()->setMaxThreadCount(MAX_THREAD_COUNT);

    QString uniqueKey = "dde-file-manager";

    bool isSingleInstance  = app.setSingleInstance(uniqueKey);

    qDebug() << isSingleInstance << commandlineUrl;

    if (isSingleInstance){
        DThemeManager::instance()->setTheme("light");

        fileManagerApp->show(commandlineUrl);
        dialogManager;

#ifdef ENABLE_PPROF
        int request = app.exec();

        ProfilerStop();
        quick_exit(request);
#else
        quick_exit(app.exec());
#endif
    }else{
        SingleApplication::newClientProcess(uniqueKey);
    }
}
