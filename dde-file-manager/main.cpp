#include "durl.h"
#include "dfmglobal.h"

#include "filemanagerapp.h"
#include "logutil.h"
#include "singleapplication.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include "widgets/commandlinemanager.h"

#include "dialogs/dialogmanager.h"
#include "shutil/fileutils.h"
#include "shutil/mimesappsmanager.h"
#include "dialogs/openwithdialog.h"
#include "controllers/appcontroller.h"
#include "widgets/singleton.h"

#include <dthememanager.h>
#include <dwindow.h>

#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include <QProcess>

#ifdef ENABLE_PPROF
#include <gperftools/profiler.h>
#endif

#define fileManagerApp Singleton<FileManagerApp>::instance()

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
#ifdef ENABLE_PPROF
    ProfilerStart("pprof.prof");
#endif
    SingleApplication::loadDXcbPlugin();
    SingleApplication::initSources();
    SingleApplication app(argc, argv);

    app.setOrganizationName(QMAKE_ORGANIZATION_NAME);
    app.setApplicationName(QMAKE_TARGET);
    app.setApplicationVersion(QMAKE_VERSION);
    app.loadTranslator();

    QFont font;
    font.setPixelSize(14);
    app.setFont(font);

    LogUtil::registerLogger();

    CommandLineManager::instance()->process();

    DUrl commandlineUrl;

    if (CommandLineManager::instance()->positionalArguments().count() > 0){
        commandlineUrl = DUrl::fromUserInput(CommandLineManager::instance()->positionalArguments().at(0));
    } else {
        commandlineUrl = DUrl::fromLocalFile(QDir::homePath());
    }

    QString uniqueKey = app.applicationName();

    bool isSingleInstance  = app.setSingleInstance(uniqueKey);
    bool isBackendRun = CommandLineManager::instance()->isSet("d");
    bool isShowPropertyRequest = CommandLineManager::instance()->isSet("p");

    qDebug() << isSingleInstance << commandlineUrl;

    if (isSingleInstance){
        DFMGlobal::installTranslator();
        DThemeManager::instance()->setTheme("light");

        if (!isBackendRun){
            fileManagerApp->show(commandlineUrl);
        }else{
            fileManagerApp;
        }

        if(isShowPropertyRequest){
            QStringList paths = CommandLineManager::instance()->positionalArguments();
            fileManagerApp->showPropertyDialog(paths);
        }

        FileUtils::setDefaultFileManager();
#ifdef ENABLE_PPROF
        int request = app.exec();

        ProfilerStop();
        quick_exit(request);
#else
        int ret = app.exec();
#ifdef AUTO_RESTART_DEAMON
        app.closeServer();
        QProcess::startDetached(QString("%1 -d").arg(QString(argv[0])));
#endif
        quick_exit(ret);
#endif
    }else{
        SingleApplication::newClientProcess(uniqueKey);
        QWidget w;
        w.setWindowFlags(Qt::FramelessWindowHint);
        w.setAttribute(Qt::WA_TranslucentBackground);
        w.resize(0, 0);
        w.show();
    }
}
