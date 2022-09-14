// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <signal.h>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <DLog>
#include <QObject>
#include "app/global.h"
#include "app/filemanagerdaemon.h"
#include "client/filemanagerclient.h"
#include "log/dfmLogManager.h"
#include "anything/anything.h"

#include "ddiskmanager.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

static void handleSIGTERM(int sig)
{
    if (qApp) {
        qApp->quit();
    }
}

int main(int argc, char *argv[])
{

    ///###: why?
    ///###: when dbus invoke a daemon the variants in the environment of daemon(s) are empty.
    ///###: So we need to set them.
    if (!qEnvironmentVariableIsSet("LANG")) {
        qputenv("LANG", "en_US.UTF8");
    }

    if (!qEnvironmentVariableIsSet("LANGUAGE")) {
        qputenv("LANGUAGE", "en_US");
    }

    if (!qEnvironmentVariableIsSet("HOME")) {
        qputenv("HOME", getpwuid(getuid())->pw_dir);
    }

    QCoreApplication a(argc, argv);

    a.setOrganizationName("deepin");

    QString logPath = "/var/log/" + QCoreApplication::organizationName() + QLatin1Char('/') +
                      QCoreApplication::applicationName() + QLatin1Char('/');
    QDir logDir(logPath);
    if (!logDir.exists()) {
        QDir().mkpath(logPath);
    }

    // handle SIGTERM signal from systemctl stop
    signal(SIGTERM, handleSIGTERM);

    initAnything();

    QDBusConnection connection = QDBusConnection::systemBus();
    DFM_NAMESPACE::DFMLogManager::setlogFilePath(logPath + QCoreApplication::applicationName() + ".log");
    DFM_NAMESPACE::DFMLogManager::registerConsoleAppender();
    DFM_NAMESPACE::DFMLogManager::registerFileAppender();

    if (!connection.interface()->isServiceRegistered(DaemonServicePath)){
        qDebug() << connection.registerService(DaemonServicePath) << "register" << DaemonServicePath << "success";
        FileManagerDaemon* daemon = new FileManagerDaemon();
        qDebug() << daemon;
        return a.exec();
    }else{
        qDebug() << "dde-file-manager-daemon is running";
        FileManagerClient* client = new FileManagerClient();
        qDebug() << client << QCoreApplication::applicationPid();
        return a.exec();
    }
}
