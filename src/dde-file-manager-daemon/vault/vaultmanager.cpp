/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#include "vaultmanager.h"
#include "dbusadaptor/vault_adaptor.h"
#include "app/policykithelper.h"
#include "vaultclock.h"

#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusVariant>
#include <QProcess>
#include <QDebug>
#include <QDateTime>


QString VaultManager::ObjectPath = "/com/deepin/filemanager/daemon/VaultManager";
QString VaultManager::PolicyKitCreateActionId = "com.deepin.filemanager.daemon.VaultManager.Create";
QString VaultManager::PolicyKitRemoveActionId = "com.deepin.filemanager.daemon.VaultManager.Remove";

VaultManager::VaultManager(QObject *parent)
    : QObject(parent)
    , QDBusContext()
    , m_curVaultClock(nullptr)
    , m_pcTime(0)
{
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_vaultAdaptor = new VaultAdaptor(this);

    // create a default vault clock.
    m_curVaultClock = new VaultClock(this);
    m_curUser = getCurrentUser();
    m_mapUserClock.insert(m_curUser, m_curVaultClock);

    // monitor system user changed.
    QDBusConnection::systemBus().connect(
                "com.deepin.dde.LockService",
                "/com/deepin/dde/LockService",
                "com.deepin.dde.LockService",
                "UserChanged",
                this,
                SLOT(sysUserChanged(QString)));

    QDBusConnection::systemBus().connect(
                  "org.freedesktop.login1",
                  "/org/freedesktop/login1",
                  "org.freedesktop.login1.Manager",
                  "PrepareForSleep",
                  this,
                  SLOT(computerSleep(bool)));
}

VaultManager::~VaultManager()
{
}

void VaultManager::sysUserChanged(const QString &curUser)
{
    if (m_curUser != curUser) {
        m_curUser = curUser;
        bool bContain = m_mapUserClock.contains(m_curUser);
        if (bContain) {
            m_curVaultClock = m_mapUserClock[m_curUser];
        } else {
            m_curVaultClock = new VaultClock(this);
            m_mapUserClock.insert(m_curUser, m_curVaultClock);
        }
    }
}

void VaultManager::setRefreshTime(quint64 time)
{
    m_curVaultClock->setRefreshTime(time);
}

quint64 VaultManager::getLastestTime() const
{
    return m_curVaultClock->getLastestTime();
}

quint64 VaultManager::getSelfTime() const
{
    return m_curVaultClock->getSelfTime();
}

bool VaultManager::checkAuthentication(QString type)
{
    bool ret = false;
    qint64 pid = 0;
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if(c.isConnected()) {
        pid = c.interface()->servicePid(message().service()).value();
    }

    if (pid){
        if (type.compare("Create") == 0) {
            ret = PolicyKitHelper::instance()->checkAuthorization(PolicyKitCreateActionId, pid);
        }else if (type.compare("Remove") == 0){
            ret = PolicyKitHelper::instance()->checkAuthorization(PolicyKitRemoveActionId, pid);
        }
    }

    if (!ret) {
        qDebug() << "Authentication failed !!";
    }
    return ret;
}

bool VaultManager::isLockEventTriggered() const
{
    return m_curVaultClock->isLockEventTriggered();
}

void VaultManager::triggerLockEvent()
{
    m_curVaultClock->triggerLockEvent();
}

void VaultManager::clearLockEvent()
{
    m_curVaultClock->clearLockEvent();
}

void VaultManager::computerSleep(bool bSleep)
{
    if (bSleep) {
        m_pcTime = QDateTime::currentSecsSinceEpoch();
    } else {
        qint64 diffTime = QDateTime::currentSecsSinceEpoch() - m_pcTime;
        if (diffTime > 0) {
            for (auto key : m_mapUserClock.keys()) {
                VaultClock *vaultClock = m_mapUserClock.value(key);
                if (vaultClock) {
                    vaultClock->addTickTime(diffTime);
                }
            }
        }
    }
}

QString VaultManager::getCurrentUser() const
{
    QString user = m_curUser;

    QDBusInterface sessionManagerIface("com.deepin.dde.LockService",
    "/com/deepin/dde/LockService",
    "com.deepin.dde.LockService",
    QDBusConnection::systemBus());

    if (sessionManagerIface.isValid()) {
        QDBusPendingCall call = sessionManagerIface.asyncCall("CurrentUser");
        call.waitForFinished();
        if (!call.isError()) {
            QDBusReply<QString> reply = call.reply();
            user = reply.value();
        }
    }

    return user;
}
