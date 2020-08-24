/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QDebug>
#include <QDBusError>
#include <QDBusConnection>
#include <QThreadPool>
#include <QPixmapCache>

#include <DLog>
#include <DApplication>

#include <unistd.h>

#include <dfmglobal.h>
#include <dfmapplication.h>

#include "util/dde/ddesession.h"

#include "config/config.h"
#include "desktop.h"
#include "view/canvasgridview.h"

#include "deventfilter.h"
#include "controllers/appcontroller.h"

// DBus
#include "filedialogmanager_adaptor.h"
#include "dbusfiledialogmanager.h"
#include "filemanager1_adaptor.h"
#include "dbusfilemanager1.h"
#include "accessible/accessiblelist.h"
#include "util/dde/desktopinfo.h"

using namespace Dtk::Core;
using namespace Dtk::Widget;

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

static QTime gTime;
int main(int argc, char *argv[])
{
    gTime.start();
    QString tmp;
    // Fixed the locale codec to utf-8
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    if (DesktopInfo().waylandDectected()) {
        qputenv("QT_WAYLAND_SHELL_INTEGRATION","kwayland-shell"); //wayland shell
        tmp += QString(" end load kwayland-shell %0").arg(gTime.elapsed());
    } else {
        DApplication::loadDXcbPlugin();//wayland下不加载xcb
        tmp += QString(" end loadDXcbPlugin %0").arg(gTime.elapsed());
    }

    DApplication app(argc, argv);
    tmp += QString(" end DApplication %0").arg(gTime.elapsed());

//    AppController::instance();
//    tmp += QString(" end AppController::instance %0").arg(gTime.elapsed());
    //QAccessible::installFactory(accessibleFactory);
 // tmp += QString(" end installFactory %0").arg(gTime.elapsed());

    bool preload = false;
    bool fileDialogOnly = false;

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

    if (fileDialogOnly && getuid() != 0) {
        // --file-dialog-only should only used by `root`.
        qDebug() << "Current UID != 0, the `--file-dialog-only` argument is ignored.";
        fileDialogOnly = false;
    }

    if (fileDialogOnly) {
        app.setQuitOnLastWindowClosed(false);
    }
    tmp += QString(" begin loadTranslator %0").arg(gTime.elapsed());
    app.loadTranslator();
    tmp += QString(" end loadTranslator %0").arg(gTime.elapsed());
    app.setOrganizationName("deepin");
    app.setApplicationDisplayName(app.translate("DesktopMain", "Desktop"));
    app.setApplicationVersion(DApplication::buildVersion((GIT_VERSION)));
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    tmp += QString(" app inited %0").arg(gTime.elapsed());
    const QString m_format = "%{time}{yyyyMMdd.HH:mm:ss.zzz}[%{type:1}][%{function:-35} %{line:-4} %{threadid} ] %{message}\n";
    DLogManager::setLogFormat(m_format);
    DLogManager::registerConsoleAppender();

    if (!preload) {
        DLogManager::registerFileAppender();
    }
    qDebug() << tmp;
    tmp.clear();
    qDebug() << "registerFileAppender " <<  gTime.elapsed();

    // init application object
    DFMApplication fmApp;
    Q_UNUSED(fmApp)

    qDebug() << "start " << app.applicationName() << app.applicationVersion() << gTime.elapsed();
    if (!preload && !fileDialogOnly) {
        QDBusConnection conn = QDBusConnection::sessionBus();

        if (!conn.registerService(DesktopServiceName)) {
            qDebug() << "registerService Failed, maybe service exist" << conn.lastError();
            exit(0x0002);
        }

        if (!conn.registerObject(DesktopServicePath, Desktop::instance(),
                                 QDBusConnection::ExportAllSlots |
                                 QDBusConnection::ExportAllSignals |
                                 QDBusConnection::ExportAllProperties)) {
            qDebug() << "registerObject Failed" << conn.lastError();
            exit(0x0003);
        }

        Desktop::instance()->initDebugDBus(conn);
    }
#if 1
    if  (!preload) {
        // Notify dde-desktop start up
        if (!fileDialogOnly) {
            Dde::Session::RegisterDdeSession();
            qDebug() << "end RegisterDdeSession" <<  gTime.elapsed();
        }

        // ---------------------------------------------------------------------------
        // ability to show file selection dialog
        if (!registerDialogDBus()) {
            qWarning() << "Register dialog dbus failed.";
            if (fileDialogOnly) {
                return 1;
            }
        }

        if (!registerFileManager1DBus()) {
            qWarning() << "Register org.freedesktop.FileManager1 DBus service is failed";
        }
    }
#endif
    qDebug() << "register desktop" <<  gTime.elapsed();

    // init pixmap cache size limit, 20MB * devicePixelRatio
    QPixmapCache::setCacheLimit(static_cast<int>(20 * 1024 * app.devicePixelRatio()));

    QThreadPool::globalInstance()->setMaxThreadCount(MAX_THREAD_COUNT);

    if (!fileDialogOnly) {
        Desktop::instance()->preInit();
        Config::instance();
        qDebug() << "Desktop inited" << gTime.elapsed();
    }

    DFMGlobal::installTranslator();

    if (!fileDialogOnly) {
        Desktop::instance()->loadData();
    }

    if (preload) {
        QTimer::singleShot(1000, &app, &QCoreApplication::quit);
    } else {
        if (!fileDialogOnly) {
#if 0 //old
            qDebug() << "begin load view" <<  gTime.elapsed();
            Desktop::instance()->Show();
#endif
            Desktop::instance()->loadView();
            qDebug() << "end load view" <<  gTime.elapsed();
        }
    }

    QTimer::singleShot(100,[](){
        qDebug() << "begin load plugin " <<  gTime.elapsed();

        AppController::instance();
        qDebug() << "end AppController::instance "<< gTime.elapsed();

        DFMGlobal::autoLoadDefaultPlugins();
        qDebug() << "end autoLoadDefaultPlugins" <<  gTime.elapsed();
        DFMGlobal::initPluginManager();
        qDebug() << "end initPluginManager" <<  gTime.elapsed();
        DFMGlobal::initMimesAppsManager();
        qDebug() << "end initMimesAppsManager" <<  gTime.elapsed();
        DFMGlobal::initDialogManager();
        qDebug() << "end initDialogManager" <<  gTime.elapsed();
        DFMGlobal::initOperatorRevocation();
        qDebug() << "end initOperatorRevocation" <<  gTime.elapsed();
        DFMGlobal::initTagManagerConnect();
        qDebug() << "end initTagManagerConnect" <<  gTime.elapsed();
        DFMGlobal::initThumbnailConnection();
        qDebug() << "end initThumbnailConnection" <<  gTime.elapsed();
        DFMGlobal::initDeviceListener();
        qDebug() << "end initDeviceListener" <<  gTime.elapsed(); //高耗时
        DFMGlobal::initGvfsMountManager();
        qDebug() << "end initGvfsMountManager and load plugin" <<  gTime.elapsed();
    });
#if 0
    qDebug() << "begin load plugin " <<  gTime.elapsed();
    DFMGlobal::autoLoadDefaultPlugins();
    qDebug() << "end autoLoadDefaultPlugins" <<  gTime.elapsed();
    DFMGlobal::initPluginManager();
    qDebug() << "end initPluginManager" <<  gTime.elapsed();
    DFMGlobal::initMimesAppsManager();
    qDebug() << "end initMimesAppsManager" <<  gTime.elapsed();
    DFMGlobal::initDialogManager();
    qDebug() << "end initDialogManager" <<  gTime.elapsed();
    DFMGlobal::initOperatorRevocation();
    qDebug() << "end initOperatorRevocation" <<  gTime.elapsed();
    DFMGlobal::initTagManagerConnect();
    qDebug() << "end initTagManagerConnect" <<  gTime.elapsed();
    DFMGlobal::initThumbnailConnection();
    qDebug() << "end initThumbnailConnection" <<  gTime.elapsed();
    DFMGlobal::initDeviceListener();
    qDebug() << "end initDeviceListener" <<  gTime.elapsed(); //高耗时
    DFMGlobal::initGvfsMountManager();
    qDebug() << "end initGvfsMountManager and load plugin" <<  gTime.elapsed();
#endif

#if 0
    if  (!preload) {
        // Notify dde-desktop start up
        if (!fileDialogOnly) {
            Dde::Session::RegisterDdeSession();
            QApplication::processEvents();
            qDebug() << "end RegisterDdeSession" <<  gTime.elapsed();
        }

        // ---------------------------------------------------------------------------
        // ability to show file selection dialog
        if (!registerDialogDBus()) {
            qWarning() << "Register dialog dbus failed.";
            if (fileDialogOnly) {
                return 1;
            }
        }

        if (!registerFileManager1DBus()) {
            qWarning() << "Register org.freedesktop.FileManager1 DBus service is failed";
        }
    }
#endif
    DFMGlobal::IsFileManagerDiloagProcess = true; // for compatibility.
    // ---------------------------------------------------------------------------

//    DEventFilter *event_filter{ new DEventFilter{&app} };
//    app.installEventFilter(event_filter);


    return app.exec();
}
