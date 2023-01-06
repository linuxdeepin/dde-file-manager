// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "util/dde/ddesession.h"
#include "config/config.h"
#include "desktop.h"
#include "view/canvasgridview.h"
#include "daemonplugin.h"
#include "app/define.h"
#include "rlog/rlog.h"

#include <controllers/appcontroller.h>
#include <dfmglobal.h>
#include <dfmapplication.h>
#include <log/dfmLogManager.h>

// DBus
#include "filedialogmanager_adaptor.h"
#include "dbusfiledialogmanager.h"
#include "filemanager1_adaptor.h"
#include "dbusfilemanager1.h"
#include "util/dde/desktopinfo.h"
#include "accessibility/acobjectlist.h"

#include <DLog>
#include <DApplication>

#include <QDebug>
#include <QDBusError>
#include <QDBusConnection>
#include <QThreadPool>
#include <QPixmapCache>
#include <QSurfaceFormat>

#include <unistd.h>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DFM_USE_NAMESPACE

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
    QString startTime = QDateTime::currentDateTime().toString("yyyyMMdd.hh:mm:ss:zzz");
    //for qt5platform-plugins load DPlatformIntegration or DPlatformIntegrationParent
    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")){
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }

    // Fixed the locale codec to utf-8
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    DApplication app(argc, argv);

    if (DesktopInfo().waylandDectected()) {
        //! 以下代码用于视频预览使用
        setenv("PULSE_PROP_media.role", "video", 1);
#ifndef __x86_64
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setDefaultFormat(format);
#endif
    } else {
        //fix bug59539 向桌面拖拽文件，进度条框无法拖动
        //根因：桌面启动时窗管未完全启动，导致xcb插件fallback到不同的分支
        //方案：设置环境变量来强制指定 使用NO_TITLEBAR
        qputenv("D_DXCB_FORCE_NO_TITLEBAR", "1");
    }

    // 集成测试标签
#ifdef ENABLE_ACCESSIBILITY
    QAccessible::installFactory(accessibleFactory);
    QAccessible::setActive(true);
#else
    QAccessible::setActive(false);
#endif

    bool preload = false;
    bool fileDialogOnly = false;
    bool noGrandSearch = false;
    for (const QString &arg : app.arguments()) {
        if (arg == "--preload") {
            preload = true;
            break;
        }
        if (arg == "--file-dialog-only") {
            fileDialogOnly = true;
            break;
        }
    }

    if (app.arguments().contains("--no-grandsearch"))
        noGrandSearch = true;

    //因bug#46452，--file-dialog-only不再判断root
//    if (fileDialogOnly && getuid() != 0) {
//        // --file-dialog-only should only used by `root`.
//        qDebug() << "Current UID != 0, the `--file-dialog-only` argument is ignored.";
//        fileDialogOnly = false;
//    }

    app.setQuitOnLastWindowClosed(false);
    app.loadTranslator();
    app.setOrganizationName("deepin");
    app.setApplicationDisplayName(app.translate("DesktopMain", "Desktop"));
    app.setApplicationVersion(DApplication::buildVersion((GIT_VERSION)));
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setProductIcon(QIcon::fromTheme("deepin-toggle-desktop"));

    const QString logFormat = "%{time}{yyyyMMdd.HH:mm:ss.zzz}[%{type:1}][%{function:-35} %{line:-4} %{threadid} ] %{message}\n";
    DFMLogManager::setLogFormat(logFormat);
    DFMLogManager::registerConsoleAppender();

    if (!preload)
        DFMLogManager::registerFileAppender();

    DFMApplication fmApp;
    Q_UNUSED(fmApp)

    qInfo() << "start " << app.applicationName() << app.applicationVersion() << startTime;
    qInfo() << "pid:" << getpid() << " preload:" << preload << "fileDialogOnly:" << fileDialogOnly;

    if (!preload && !fileDialogOnly) {
        QDBusConnection conn = QDBusConnection::sessionBus();

        if (!conn.registerService(DesktopServiceName)) {
            qCritical() << "registerService Failed, maybe service exist" << conn.lastError();
            exit(0x0002);
        }

        auto registerOptions = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
        if (!conn.registerObject(DesktopServicePath, Desktop::instance(), registerOptions)) {
            qCritical() << "registerObject Failed" << conn.lastError();
            exit(0x0003);
        }
    }

    if (!preload) {
        // Notify dde-desktop start up
        if (!fileDialogOnly)
            Dde::Session::RegisterDdeSession();

        // ability to show file selection dialog
        if (!registerDialogDBus()) {
            qWarning() << "Register dialog dbus failed.";
            if (fileDialogOnly)
                return 1;
        }

        if (!registerFileManager1DBus())
            qWarning() << "Register org.freedesktop.FileManager1 DBus service is failed";
    }

    // init pixmap cache size limit, 20MB * devicePixelRatio
    QPixmapCache::setCacheLimit(static_cast<int>(20 * 1024 * app.devicePixelRatio()));
    QThreadPool::globalInstance()->setMaxThreadCount(MAX_THREAD_COUNT);

    if (!fileDialogOnly) {
        Desktop::instance()->preInit();
        Config::instance();
    }

    DFMGlobal::installTranslator();
    if (!fileDialogOnly)
        Desktop::instance()->loadData();

    if (preload) {
        QTimer::singleShot(1000, &app, &QCoreApplication::quit);
    } else {
        if (!fileDialogOnly) {
            Desktop::instance()->loadView();

            if (!noGrandSearch) {
                // 4s后启动全局搜索
                QTimer::singleShot(4000, Desktop::instance(), []() {
                   qInfo() << "try to start grand search daemon...";
                    GrandSearch::DaemonPlugin *srv = new GrandSearch::DaemonPlugin(qApp);
                    if (srv->initialize()) {
                        if (srv->start()) {
                            QObject::connect(qApp, &DApplication::aboutToQuit, srv, &GrandSearch::DaemonPlugin::stop);
                            qInfo() << "grand search daemo started successfully.";
                            return;
                        } else {
                            qCritical() << "started grand search failed.";
                        }
                    } else {
                        qWarning() << "initialize grand search failed.";
                    }
                    delete srv;
                });
            }
        }
    }

    QTimer::singleShot(100, []() {
        AppController::instance();
        DFMGlobal::autoLoadDefaultPlugins();
        DFMGlobal::initPluginManager();
        DFMGlobal::initMimesAppsManager();
        DFMGlobal::initDialogManager();
        DFMGlobal::initOperatorRevocation();
        DFMGlobal::initTagManagerConnect();
        DFMGlobal::initThumbnailConnection();
        DFMGlobal::initDeviceListener();
        DFMGlobal::initGvfsMountManager();
        DFMGlobal::initBluetoothManager();
        DFMGlobal::initRootFileManager();
        DFMGlobal::setInitAppOver();
        DFMGlobal::initVaultDbusResponse();
        DFMGlobal::initRlogManager();
    });
    DFMGlobal::initEmblemPluginManagerConnection();

    DFMGlobal::IsFileManagerDiloagProcess = true; // for compatibility.

    qInfo() << "RLog init start!";
    rlog->init();
    qInfo() << "RLog init finished!";

    int ret = app.exec();
    qInfo() << "exit: " << ret << "pid:" << getpid();
    return ret;
}
