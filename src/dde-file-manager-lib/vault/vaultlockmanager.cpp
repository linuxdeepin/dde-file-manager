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

#include "vaultlockmanager.h"
#include "dfileservices.h"
#include "dfmsettings.h"
#include "dfmapplication.h"
#include "controllers/vaultcontroller.h"
#include "controllers/vaulterrorcode.h"
#include "../app/define.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/dtaskdialog.h"
#include "interfaceactivevault.h"
#include "vault/vaulthelper.h"
#include "dbusinterface/vault_interface.h"
#include "vault/vaulthelper.h"

#include <unistd.h>

#define VAULT_AUTOLOCK_KEY      "AutoLock"
#define VAULT_GROUP             "Vault/AutoLock"

/**
 * @brief The VaultLockManagerPrivate class
 */
class VaultLockManagerPrivate
{
public:
    explicit VaultLockManagerPrivate(VaultLockManager *qq);

    VaultInterface *m_vaultInterface = nullptr; // dbus interface
    VaultLockManager::AutoLockState m_autoLockState; // auto lock state
    QTimer m_alarmClock; // auto lock clock
    bool m_isCacheTimeReloaded;

    VaultLockManager *q_ptr;

    Q_DECLARE_PUBLIC(VaultLockManager)
};

VaultLockManagerPrivate::VaultLockManagerPrivate(VaultLockManager *qq)
    : m_autoLockState(VaultLockManager::Never)
    , m_isCacheTimeReloaded(false)
    , q_ptr(qq)
{
}

VaultLockManager &VaultLockManager::getInstance()
{
    static VaultLockManager instance;
    return instance;
}

VaultLockManager::VaultLockManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new VaultLockManagerPrivate(this))
{
    Q_D(VaultLockManager);

    d->m_vaultInterface = new VaultInterface("com.deepin.filemanager.daemon",
                                             "/com/deepin/filemanager/daemon/VaultManager",
                                             QDBusConnection::systemBus(),
                                             this);

    if (!isValid()) {
        qDebug() << d->m_vaultInterface->lastError().message();
        return;
    }

    //! 自动锁计时处理
    connect(&d->m_alarmClock, &QTimer::timeout, this, &VaultLockManager::processAutoLock);
    d->m_alarmClock.setInterval(1000);

    connect(VaultController::ins(), &VaultController::signalLockVault, this,  &VaultLockManager::slotLockVault);
    connect(VaultController::ins(), &VaultController::signalUnlockVault, this,  &VaultLockManager::slotUnlockVault);

    loadConfig();

    // monitor screen lock event.
    QDBusConnection::sessionBus().connect(
        "org.freedesktop.FileManager1",
        "/org/freedesktop/FileManager1",
        "org.freedesktop.FileManager1",
        "lockEventTriggered",
        this,
        SLOT(slotLockEvent(QString)));
}

void VaultLockManager::loadConfig()
{
    VaultLockManager::AutoLockState state = VaultLockManager::Never;
    QVariant var = DFMApplication::genericSetting()->value(VAULT_GROUP, VAULT_AUTOLOCK_KEY);
    if (var.isValid()) {
        state = static_cast<VaultLockManager::AutoLockState>(var.toInt());
    }
    autoLock(state);
}

void VaultLockManager::resetConfig()
{
    autoLock(VaultLockManager::Never);
}

VaultLockManager::AutoLockState VaultLockManager::autoLockState() const
{
    D_DC(VaultLockManager);

    return d->m_autoLockState;
}

bool VaultLockManager::autoLock(VaultLockManager::AutoLockState lockState)
{
    Q_D(VaultLockManager);

    d->m_autoLockState = lockState;

    if (d->m_autoLockState == Never) {
        d->m_alarmClock.stop();
    } else {
        if (d->m_isCacheTimeReloaded) {
            refreshAccessTime();
        }

        d->m_alarmClock.start();
    }
    d->m_isCacheTimeReloaded = true;

    DFMApplication::genericSetting()->setValue(VAULT_GROUP, VAULT_AUTOLOCK_KEY, lockState);

    return true;
}

void VaultLockManager::refreshAccessTime()
{
    if (isValid()) {
        quint64 curTime = dbusGetSelfTime();
        dbusSetRefreshTime(static_cast<quint64>(curTime));
    }
}

bool VaultLockManager::checkAuthentication(QString type)
{
    Q_D(VaultLockManager);

    bool res = false;
    if (d->m_vaultInterface->isValid()) {
        QDBusPendingReply<bool> reply = d->m_vaultInterface->checkAuthentication(type);
        reply.waitForFinished();
        if (reply.isError()) {
            qDebug() << reply.error().message();
        } else {
            res = reply.value();
        }
    }
    return res;
}

void VaultLockManager::processAutoLock()
{
    Q_D(VaultLockManager);

    VaultController *controller = VaultController::ins();
    if (controller->state() != VaultController::Unlocked
            || d->m_autoLockState == Never) {

        return;
    }

    quint64 lastAccessTime = dbusGetLastestTime();

    quint64 curTime = dbusGetSelfTime();

    quint64 interval = curTime - lastAccessTime;
    quint32 threshold = d->m_autoLockState * 60;

#ifdef AUTOLOCK_TEST
    qDebug() << "vault autolock countdown > " << interval;
#endif

    if (interval > threshold) {
        qDebug() << "-----------enter interval > threshold-------------";
        VaultHelper::killVaultTasks();
        qDebug() << "---------------begin lockVault---------------";
        controller->lockVault();
        qDebug() << "---------------leave lockVault---------------";
    }
}

void VaultLockManager::slotLockVault(int msg)
{
    Q_D(VaultLockManager);

    if (static_cast<ErrorCode>(msg) == ErrorCode::Success) {
        d->m_alarmClock.stop();
    } else {
        qDebug() << "vault cannot lock";
    }
}

void VaultLockManager::slotUnlockVault(int msg)
{
    Q_D(VaultLockManager);

    if (static_cast<ErrorCode>(msg) == ErrorCode::Success) {
        autoLock(d->m_autoLockState);
    }
}

void VaultLockManager::processLockEvent()
{
    // lock vault.
    VaultHelper::killVaultTasks();
    VaultController::ins()->lockVault();
}

void VaultLockManager::slotLockEvent(const QString &user)
{
    char *loginUser = getlogin();
    if (user == loginUser) {
        processLockEvent();
    }
}

bool VaultLockManager::isValid() const
{
    D_DC(VaultLockManager);

    bool bValid = false;

    if (d->m_vaultInterface->isValid()) {
        QDBusPendingReply<quint64> reply = d->m_vaultInterface->getLastestTime();
        reply.waitForFinished();
        bValid = !reply.isError();
    }
    return bValid;
}

void VaultLockManager::dbusSetRefreshTime(quint64 time)
{
    Q_D(VaultLockManager);

    if (d->m_vaultInterface->isValid()) {
        QDBusPendingReply<> reply = d->m_vaultInterface->setRefreshTime(time);
        reply.waitForFinished();
        if (reply.isError()) {
            qDebug() << reply.error().message();
        }
    }
}

quint64 VaultLockManager::dbusGetLastestTime() const
{
    D_DC(VaultLockManager);

    quint64 latestTime = 0;
    if (d->m_vaultInterface->isValid()) {
        QDBusPendingReply<quint64> reply = d->m_vaultInterface->getLastestTime();
        reply.waitForFinished();
        if (reply.isError()) {
            qDebug() << reply.error().message();
        } else {
            latestTime = reply.value();
        }
    }
    return latestTime;
}

quint64 VaultLockManager::dbusGetSelfTime() const
{
    D_DC(VaultLockManager);

    quint64 selfTime = 0;
    if (d->m_vaultInterface->isValid()) {
        QDBusPendingReply<quint64> reply = d->m_vaultInterface->getSelfTime();
        reply.waitForFinished();
        if (reply.isError()) {
            qDebug() << reply.error().message();
        } else {
            selfTime = reply.value();
        }
    }
    return selfTime;
}
