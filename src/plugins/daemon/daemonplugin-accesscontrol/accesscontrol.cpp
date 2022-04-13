/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "accesscontrol.h"
#include "dbusadaptor/accesscontrolmanager_adaptor.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

DAEMONPAC_USE_NAMESPACE

bool AccessControl::start()
{
    if (!isDaemonServiceRegistered())
        return false;

    watcher.reset(new DFMIO::DLocalWatcher(QUrl::fromLocalFile("/home")));

    onFileCreatedInHomePath();
    initConnect();
    initDBusInterce();
    return true;
}

bool AccessControl::isDaemonServiceRegistered()
{
    static constexpr char kEnvNameOfDaemonRegistered[] { "DAEMON_SERVICE_REGISTERED" };

    QByteArray registered { qgetenv(kEnvNameOfDaemonRegistered) };
    qInfo() << "Env DAEMON_SERVICE_REGISTERED is: " << registered;
    if (QString::fromLocal8Bit(registered) == "TRUE")
        return true;

    return false;
}

void AccessControl::initDBusInterce()
{
    accessControlManager.reset(new AccessControlDBus);
    Q_UNUSED(new AccessControlManagerAdaptor(accessControlManager.data()));
    if (!QDBusConnection::systemBus().registerObject("/com/deepin/filemanager/daemon/AccessControlManager", accessControlManager.data())) {
        qWarning("Cannot register the \"/com/deepin/filemanager/daemon/AccessControlManager\" object.\n");
        accessControlManager.reset(nullptr);
        return;
    }
}

void AccessControl::initConnect()
{
    if (Q_UNLIKELY((watcher.isNull()))) {
        qWarning() << "Wathcer is invliad";
        return;
    }

    connect(watcher.data(), &DFMIO::DLocalWatcher::fileAdded, this, [this](const QUrl &url) {
        qInfo() << "File: " << url << " has been created";
        onFileCreatedInHomePath();
    });
    watcher->start();
}

void AccessControl::onFileCreatedInHomePath()
{
    QDir homeDir("/home");
    QStringList dirNames = homeDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs);
    dirNames.append("root");   // root 用户不在/home下，但设备会挂载到/media/root
    for (const QString &dirName : dirNames) {
        // /media/[UserName] 为默认挂载的基路径，预先从创建此目录，目的是为了确保该路径其他用户能够访问
        QString mountBaseName = QString("/media/%1").arg(dirName);
        QDir mountDir(mountBaseName);
        if (!mountDir.exists()) {
            qInfo() << mountBaseName << "not exists";
            if (QDir().mkpath(mountBaseName)) {
                qInfo() << "create" << mountBaseName << "success";
                struct stat fileStat;
                QByteArray nameBytes(mountBaseName.toUtf8());
                stat(nameBytes.data(), &fileStat);
                chmod(nameBytes.data(), (fileStat.st_mode | S_IRUSR | S_IRGRP | S_IROTH));
            }
        }
        // ACL
        QString aclCmd = QString("setfacl -m o:rx %1").arg(mountBaseName);
        QProcess::execute(aclCmd);
        qInfo() << "acl:" << aclCmd;
    }
}
