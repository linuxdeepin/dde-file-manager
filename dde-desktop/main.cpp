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

#include "util/dde/ddesession.h"

#include "config/config.h"
#include "desktop.h"
#include "view/canvasgridview.h"

using namespace Dtk::Core;
using namespace Dtk::Widget;

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

    app.setOrganizationName("deepin");
    app.setApplicationName("dde-desktop");
    app.setApplicationVersion((GIT_VERSION));
    app.setTheme("light");
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    const QString m_format = "%{time}{yyyyMMdd.HH:mm:ss.zzz}[%{type:1}][%{function:-35} %{line:-4} %{threadid} ] %{message}\n";
    DLogManager::setLogFormat(m_format);
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    app.loadTranslator();

    qDebug() << "start " << app.applicationName() << app.applicationVersion();

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

    // init pixmap cache size limit, 20MB * devicePixelRatio
    QPixmapCache::setCacheLimit(20 * 1024 * 1024 * app.devicePixelRatio());

    QThreadPool::globalInstance()->setMaxThreadCount(MAX_THREAD_COUNT);
    Config::instance();

    Desktop::instance()->loadData();
    Desktop::instance()->Show();
    Desktop::instance()->loadView();

    DFMGlobal::installTranslator();
    DFMGlobal::autoLoadDefaultPlugins();
    DFMGlobal::autoLoadDefaultMenuExtensions();
    DFMGlobal::initPluginManager();
    DFMGlobal::initMimesAppsManager();
    DFMGlobal::initDialogManager();
    DFMGlobal::initGvfsMountManager();
    DFMGlobal::initDeviceListener();
    DFMGlobal::autoMountAllDisks();

    // Notify dde-desktop start up
    Dde::Session::RegisterDdeSession();

    return app.exec();
}
