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

#include <dfmglobal.h>
#include <dfmapplication.h>

#include "util/dde/ddesession.h"

#include "config/config.h"
#include "desktop.h"
#include "view/canvasgridview.h"

#include "deventfilter.h"

// DBus
#include "filedialogmanager_adaptor.h"
#include "dbusfiledialogmanager.h"
#include "filemanager1_adaptor.h"
#include "dbusfilemanager1.h"

#include <QApplication>

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

int main(int argc, char *argv[])
{
    // Fixed the locale codec to utf-8
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    if (QFile("/usr/lib/dde-desktop/plugins/platform/libdxcb.so").exists()) {
        qDebug() << "load dxcb from local path: /usr/lib/dde-desktop/plugins/platform/libdxcb.so";
        qputenv("QT_QPA_PLATFORM_PLUGIN_PATH", "/usr/lib/dde-desktop/plugins/platform");
    }

    DApplication::loadDXcbPlugin();

    DApplication app(argc, argv);

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

    if (fileDialogOnly) {
        app.setQuitOnLastWindowClosed(false);
    }

    app.setOrganizationName("deepin");
    app.setApplicationName("dde-desktop");
    app.setApplicationVersion(DApplication::buildVersion((GIT_VERSION)));
    app.setTheme("light");
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    const QString m_format = "%{time}{yyyyMMdd.HH:mm:ss.zzz}[%{type:1}][%{function:-35} %{line:-4} %{threadid} ] %{message}\n";
    DLogManager::setLogFormat(m_format);
    DLogManager::registerConsoleAppender();

    if (!preload) {
        DLogManager::registerFileAppender();
    }

    app.loadTranslator();

    // init application object
    DFMApplication fmApp;
    Q_UNUSED(fmApp)

    qDebug() << "start " << app.applicationName() << app.applicationVersion();

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

    // init pixmap cache size limit, 20MB * devicePixelRatio
    QPixmapCache::setCacheLimit(20 * 1024 * app.devicePixelRatio());

    QThreadPool::globalInstance()->setMaxThreadCount(MAX_THREAD_COUNT);

    if (!fileDialogOnly) {
        Config::instance();
    }

    DFMGlobal::installTranslator();

    if (!fileDialogOnly) {
        Desktop::instance()->loadData();
    }

    if (preload) {
        QTimer::singleShot(1000, &app, &QCoreApplication::quit);
    } else {
        if (!fileDialogOnly) {
            Desktop::instance()->Show();
            Desktop::instance()->loadView();
        }
    }

    DFMGlobal::autoLoadDefaultPlugins();
    DFMGlobal::autoLoadDefaultMenuExtensions();
    DFMGlobal::initPluginManager();
    DFMGlobal::initMimesAppsManager();
    DFMGlobal::initDialogManager();
    DFMGlobal::initOperatorRevocation();
    DFMGlobal::initTagManagerConnect();
    DFMGlobal::initThumbnailConnection();

    if  (!preload) {
        // Notify dde-desktop start up
        if (!fileDialogOnly) {
            Dde::Session::RegisterDdeSession();
        }

        // ---------------------------------------------------------------------------
        // ability to show file selection dialog
        if (!registerDialogDBus()) {
            qWarning() << "Register dialog dbus failed.";
            return 1;
        }

        if (!registerFileManager1DBus()) {
            qWarning() << "Register org.freedesktop.FileManager1 DBus service is failed";
        }
    }

    DFMGlobal::IsFileManagerDiloagProcess = true; // for compatibility.
    // ---------------------------------------------------------------------------

    DEventFilter *event_filter{ new DEventFilter{&app} };
    app.installEventFilter(event_filter);

    return app.exec();
}
