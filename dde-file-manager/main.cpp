#include "durl.h"
#include "dfmglobal.h"
#include "dabstractfileinfo.h"
#include "dfileservices.h"
#include "dfmeventdispatcher.h"

#include "filemanagerapp.h"
#include "logutil.h"
#include "singleapplication.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include "commandlinemanager.h"

#include "dialogs/dialogmanager.h"
#include "shutil/fileutils.h"
#include "shutil/mimesappsmanager.h"
#include "dialogs/openwithdialog.h"
#include "controllers/appcontroller.h"
#include "singleton.h"
#include "gvfs/gvfsmountmanager.h"

// DBus
#include "filedialogmanager_adaptor.h"
#include "dbusfiledialogmanager.h"
#include "filemanager1_adaptor.h"
#include "dbusfilemanager1.h"

#include <dthememanager.h>
#include <dwindow.h>

#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include <QProcess>
#include <QLocalSocket>

#ifdef ENABLE_PPROF
#include <gperftools/profiler.h>
#endif

#define fileManagerApp FileManagerApp::instance()

DWIDGET_USE_NAMESPACE

static bool registerDialogDBus()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\n"
                 "Please check your system settings and try again.\n");
        return false;
    }

    // add our D-Bus interface and connect to D-Bus
    if (!QDBusConnection::sessionBus().registerService("com.deepin.filemanager.filedialog")) {
        qWarning("Cannot register the \"com.deepin.filemanager.filedialog\" service.\n");
        return false;
    }

    DBusFileDialogManager *manager = new DBusFileDialogManager();
    Q_UNUSED(new FiledialogmanagerAdaptor(manager));

    if (!QDBusConnection::sessionBus().registerObject("/com/deepin/filemanager/filedialogmanager", manager)) {
        qWarning("Cannot register to the D-Bus object: \"/com/deepin/filemanager/filedialogmanager\"\n");
        manager->deleteLater();
        return false;
    }

    return true;
}

static bool registerFileManager1DBus()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\n"
                 "Please check your system settings and try again.\n");
        return false;
    }

    // add our D-Bus interface and connect to D-Bus
    if (!QDBusConnection::sessionBus().registerService("org.freedesktop.FileManager1")) {
        qWarning("Cannot register the \"org.freedesktop.FileManager1\" service.\n");
        return false;
    }

    DBusFileManager1 *manager = new DBusFileManager1();
    Q_UNUSED(new FileManager1Adaptor(manager));

    if (!QDBusConnection::sessionBus().registerObject("/org/freedesktop/FileManager1", manager)) {
        qWarning("Cannot register to the D-Bus object: \"/org/freedesktop/FileManager1\"\n");
        manager->deleteLater();
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
#ifdef ENABLE_PPROF
    ProfilerStart("pprof.prof");
#endif

    // Fixed the locale codec to utf-8
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    SingleApplication::loadDXcbPlugin();
    SingleApplication::initSources();
    SingleApplication app(argc, argv);

    app.setOrganizationName(QMAKE_ORGANIZATION_NAME);
    app.setApplicationName(QMAKE_TARGET);
    app.loadTranslator();
    app.setApplicationDisplayName(app.translate("Application", "Deepin File Manager"));
    app.setApplicationVersion(QMAKE_VERSION);
    QString icon(":/images/images/dde-file-manager_96.png");
    app.setProductIcon(QPixmap(icon));
    app.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/" + qApp->applicationName());
    app.setApplicationDescription(app.translate("Application", "File Manager is a file management tool independently "
                                                               "developed by Deepin Technology, featured with searching, "
                                                               "copying, trash, compression/decompression, file property "
                                                               "and other file management functions."));

#ifdef DISABLE_QUIT_ON_LAST_WINDOW_CLOSED
    app.setQuitOnLastWindowClosed(false);
#endif

    DFMGlobal::installTranslator();
    DThemeManager::instance()->setTheme("light");

    LogUtil::registerLogger();

    CommandLineManager::instance()->process();

    // show file selection dialog
    if (CommandLineManager::instance()->isSet("f")) {
        if (!registerDialogDBus()) {
            qWarning() << "Register dialog dbus failed.";

            return 1;
        }

        if (!registerFileManager1DBus()) {
            qWarning() << "Register org.freedesktop.FileManager1 DBus service is failed";
        }
        DFMGlobal::IsFileManagerDiloagProcess = true;
        app.setQuitOnLastWindowClosed(false);
        fileManagerApp;
        gvfsMountManager->setAutoMountSwitch(false);

        // initialize the QWidget public data
        // Speed up the file choose dialog window pops up the speed
        QWidget *w = new QWidget();
        w->createWinId();
        w->deleteLater();

        return app.exec();
    }

    if (CommandLineManager::instance()->isSet("h") || CommandLineManager::instance()->isSet("v")) {
        return app.exec();
    }

    QString uniqueKey = app.applicationName();

    bool isSingleInstance  = app.setSingleInstance(uniqueKey);

    if (isSingleInstance) {
        // init app
        Q_UNUSED(FileManagerApp::instance())

        if (CommandLineManager::instance()->isSet("d")) {
            fileManagerApp;
#ifdef AUTO_RESTART_DEAMON
            QWidget w;
            w.setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
            w.setAttribute(Qt::WA_TranslucentBackground);
            w.resize(0, 0);
            w.show();
#endif
        } else {
            CommandLineManager::instance()->processCommand();
        }

        FileUtils::setDefaultFileManager();
#ifdef ENABLE_PPROF
        int request = app.exec();

        ProfilerStop();

        return request;
#else
        int ret = app.exec();
#ifdef AUTO_RESTART_DEAMON
        app.closeServer();
        QProcess::startDetached(QString("%1 -d").arg(QString(argv[0])));
#endif
        return ret;
#endif
    } else {
        QByteArray data;
        bool is_set_get_monitor_files = false;

        for (const QString &arg : app.arguments()) {
            if (arg == "--get-monitor-files")
                is_set_get_monitor_files = true;

            if (!arg.startsWith("-") && QFile::exists(arg))
                data.append(QDir(arg).absolutePath().toLocal8Bit().toBase64());
            else
                data.append(arg.toLocal8Bit().toBase64());

            data.append(' ');
        }

        if (!data.isEmpty())
            data.chop(1);

        QLocalSocket *socket = SingleApplication::newClientProcess(uniqueKey, data);
        QWidget w;
        w.setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
        w.setAttribute(Qt::WA_TranslucentBackground);
        w.resize(1, 1);
        w.show();

        if (is_set_get_monitor_files && socket->error() == QLocalSocket::UnknownSocketError) {
            socket->waitForReadyRead();

            for (const QByteArray &i : socket->readAll().split(' '))
                qDebug() << QString::fromLocal8Bit(QByteArray::fromBase64(i));
        }

        return 0;
    }
}
