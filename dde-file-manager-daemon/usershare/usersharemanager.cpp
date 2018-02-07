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

#include "usersharemanager.h"
#include "dbusservice/dbusadaptor/usershare_adaptor.h"
#include <QDBusConnection>
#include <QDBusVariant>
#include <QProcess>
#include <QDebug>


QString UserShareManager::ObjectPath = "/com/deepin/filemanager/daemon/UserShareManager";

UserShareManager::UserShareManager(QObject *parent) : QObject(parent)
{
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_userShareAdaptor = new UserShareAdaptor(this);
}

UserShareManager::~UserShareManager()
{

}

bool UserShareManager::addGroup(const QString &groupName)
{
    QStringList args;
    args << groupName;
    bool ret = QProcess::startDetached("/usr/sbin/groupadd", args);
    qDebug() << "groupadd" << groupName << ret;
    return ret;
}

bool UserShareManager::addUserToGroup(const QString &userName, const QString &groupName)
{
    addGroup(groupName);
    QStringList args;
    args << userName << groupName;
    bool ret = QProcess::startDetached("/usr/sbin/adduser", args);
    qDebug() << "adduser" << userName << groupName << ret;
    return ret;
}

bool UserShareManager::setUserSharePassword(const QString &username, const QString &passward)
{
    qDebug() << username << passward;
    QStringList args;
    args << "-a" << username << "-s";
    QProcess p;
    p.start("smbpasswd", args);
    p.write(passward.toStdString().c_str());
    p.write("\n");
    p.write(passward.toStdString().c_str());
    p.closeWriteChannel();
    bool ret = p.waitForFinished();
    qDebug() << p.readAll() << p.readAllStandardError() << p.readAllStandardOutput();
    return ret;
}

bool UserShareManager::restartSambaService()
{
    QStringList args;
    args << "restart";
    bool ret = QProcess::startDetached("/usr/sbin/smbd", args);
    qDebug() << "smbd restart" << ret;
    return ret;
}

