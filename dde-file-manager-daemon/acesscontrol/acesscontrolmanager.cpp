/*
 * Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     zhangsheng <zhangsheng@uniontech.com>
 *
 * Maintainer: zhangsheng <zhangsheng@uniontech.com>
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

#include "acesscontrolmanager.h"
#include <QDBusConnection>
#include <QDBusVariant>
#include <QProcess>
#include <QDebug>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mntent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "dfilesystemwatcher.h"

#include "app/policykithelper.h"
#include "dbusservice/dbusadaptor/acesscontrol_adaptor.h"

QString AcessControlManager::ObjectPath = "/com/deepin/filemanager/daemon/AcessControlManager";
QString AcessControlManager::PolicyKitActionId = "com.deepin.filemanager.daemon.AcessControlManager";

AcessControlManager::AcessControlManager(QObject *parent)
    : QObject(parent)
    , QDBusContext(),
      m_watcher(new DFileSystemWatcher(this))
{
    if (!QDBusConnection::systemBus().registerObject(ObjectPath, this)) {
        qFatal("AcessControlManager Register Object Failed.");
    }
    m_diskMnanager = new DDiskManager(this);
    m_diskMnanager->setWatchChanges(true);

    m_watcher->addPath("/home");
    onFileCreated("/home", "root");
    initConnect();
}

AcessControlManager::~AcessControlManager()
{

}

void AcessControlManager::initConnect()
{
    connect(m_diskMnanager, &DDiskManager::mountAdded, this, &AcessControlManager::chmodMountpoints);
    connect(m_watcher, &DFileSystemWatcher::fileCreated, this, &AcessControlManager::onFileCreated);
}

bool AcessControlManager::checkAuthentication()
{
    bool ret = false;
    qint64 pid = 0;
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if(c.isConnected()) {
       pid = c.interface()->servicePid(message().service()).value();
    }

    if (pid){
        ret = PolicyKitHelper::instance()->checkAuthorization(PolicyKitActionId, pid);
    }

    if (!ret) {
        qWarning() << "Authentication failed !!";
        qWarning() << "failed pid: " << pid;
        qWarning() << "failed policy id:" << PolicyKitActionId;
    }
    return ret;
}


/**
 * @brief 设备挂载后，为挂载路径添加写权限
 *        一些设备被 root 挂载，对其他用户没写权限，导致无法正常操作（相关 bug-60788）
 * @param blockDevicePath
 * @param mountPoint
 */
void AcessControlManager::chmodMountpoints(const QString &blockDevicePath, const QByteArray &mountPoint)
{
    Q_UNUSED(blockDevicePath);
    qInfo() << "chmod ==>" << mountPoint;
    struct stat fileStat;
    stat(mountPoint.data(), &fileStat);
    chmod(mountPoint.data(), (fileStat.st_mode | S_IWUSR | S_IWGRP | S_IWOTH));
}


void AcessControlManager::onFileCreated(const QString &path, const QString &name)
{
    Q_UNUSED(path)
    Q_UNUSED(name)

    QDir homeDir("/home");
    QStringList dirNames = homeDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs);
    dirNames.append("root"); // root 用户不在/home下，但设备会挂载到/media/root
    for (const QString &dirName : dirNames) {
        // /media/[UserName] 为默认挂载的基路径，预先从创建此目录，目的是为了确保该路径其他用户能够访问
        QString mountBaseName = QString("/media/%1").arg(dirName);
        QDir mountDir(mountBaseName);
        if (!mountDir.exists()) {
            qInfo() << mountBaseName << "not exists";
            if (QDir().mkpath(mountBaseName)) {
                qInfo() << "create" << mountBaseName << "success";
                struct stat fileStat;
                stat(mountBaseName.toUtf8().data(), &fileStat);
                chmod(mountBaseName.toUtf8().data(), (fileStat.st_mode | S_IRUSR | S_IRGRP | S_IROTH));
            }
        }
        // ACL
        QString aclCmd = QString("setfacl -m o:rx %1").arg(mountBaseName);
        QProcess::execute(aclCmd);
        qInfo() << "acl:" << aclCmd;
    }

}
