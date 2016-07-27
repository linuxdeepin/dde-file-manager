#include "durl.h"

#include "app/global.h"
#include "app/logutil.h"
#include "app/filemanagerapp.h"
#include "app/singleapplication.h"

#include "widgets/commandlinemanager.h"

#include "../dialogs/dialogmanager.h"

#include "../shutil/mimesappsmanager.h"
#include "../dialogs/openwithdialog.h"

#include "widgets/singleton.h"

#include <dthememanager.h>
#include <dwindow.h>

#include <QApplication>
#include <QDebug>
#include <QThreadPool>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>

#include "xdndworkaround.h"

#ifdef ENABLE_PPROF
#include <gperftools/profiler.h>
#endif

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(dui_theme_dark);
    Q_INIT_RESOURCE(dui_theme_light);
#ifdef ENABLE_PPROF
    ProfilerStart("pprof.prof");
#endif
    SingleApplication app(argc, argv);

    app.setOrganizationName(QMAKE_ORGANIZATION_NAME);
    app.setApplicationDisplayName(QObject::tr("File Manager"));
    app.setApplicationName(QMAKE_TARGET);
    app.setApplicationVersion(QMAKE_VERSION);

    LogUtil::registerLogger();

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

    QString uniqueKey = app.applicationName();

    bool isSingleInstance  = app.setSingleInstance(uniqueKey);

    qDebug() << isSingleInstance << commandlineUrl;

    if (isSingleInstance){
        QTranslator translator;

        if (translator.load(APPSHAREDIR"/translations/" + app.applicationName() +"_" + QLocale::system().name()))
            app.installTranslator(&translator);

        QTranslator translator_qt;

        if (translator_qt.load(QLibraryInfo::location(QLibraryInfo::TranslationsPath) + "/qt_" + QLocale::system().name() + ".qm"))
            app.installTranslator(&translator_qt);

        DThemeManager::instance()->setTheme("light");

        /// fix Qt drag drop to google chrome bug
        new XdndWorkaround();

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
