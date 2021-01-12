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

#include "basejob.h"
#include "app/policykithelper.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>

BaseJob::BaseJob(QObject *parent) : QObject(parent)
{

}

BaseJob::~BaseJob()
{
    QDBusConnection::systemBus().unregisterObject(m_objectPath);
}

qint64 BaseJob::getClientPid()
{
    qint64 pid = 0;
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if(c.isConnected())
    {
       pid = c.interface()->servicePid(message().service()).value();
    }
    return pid;
}

bool BaseJob::checkAuthorization(const QString &actionId, qint64 applicationPid)
{
    bool isAuthenticationSucceeded(false);
    qint64 pid = getClientPid();
    if (pid){
        isAuthenticationSucceeded = PolicyKitHelper::instance()->checkAuthorization(actionId, applicationPid);
    }
    return isAuthenticationSucceeded;
}

QString BaseJob::objectPath() const
{
    return m_objectPath;
}

void BaseJob::setObjectPath(const QString &objectPath)
{
    m_objectPath = objectPath;
}

bool BaseJob::registerObject()
{
    bool result = QDBusConnection::systemBus().registerObject(objectPath(), this);
    if (!result){
        deleteLater();
    }
    return result;
}


