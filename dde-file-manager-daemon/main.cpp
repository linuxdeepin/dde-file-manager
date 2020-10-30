/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

#include "ddiskmanager.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>


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
