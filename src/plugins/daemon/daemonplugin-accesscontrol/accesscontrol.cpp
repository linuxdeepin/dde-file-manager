// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    watcher.reset(new DFMIO::DWatcher(QUrl::fromLocalFile("/home")));

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

    connect(watcher.data(), &DFMIO::DWatcher::fileAdded, this, [this](const QUrl &url) {
        qInfo() << "/home/userpath has been created";
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
            if (QDir().mkpath(mountBaseName)) {
                qInfo() << "done to create /media/anyuser folder";
                struct stat fileStat;
                QByteArray nameBytes(mountBaseName.toUtf8());
                stat(nameBytes.data(), &fileStat);
                chmod(nameBytes.data(), (fileStat.st_mode | S_IRUSR | S_IRGRP | S_IROTH));
            }
        }
        // ACL
        QString aclCmd = QString("setfacl -m o:rx %1").arg(mountBaseName);
        QProcess::execute(aclCmd);
        qInfo() << "acl the /media/anyuser folder";
    }
}
