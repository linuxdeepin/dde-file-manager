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

#include "vaultlockmanager.h"
#include "dfileservices.h"
#include "controllers/vaultcontroller.h"

#include "../dde-file-manager-daemon/dbusservice/dbusinterface/vault_interface.h"

VaultLockManager &VaultLockManager::getInstance()
{
    static VaultLockManager instance;
    return instance;
}

VaultLockManager::VaultLockManager(QObject *parent) : QObject(parent)
  , m_lockState(VaultLockManager::Never)
{
    m_vaultInterface = new VaultInterface("com.deepin.filemanager.daemon",
                                          "/com/deepin/filemanager/daemon/VaultManager",
                                          QDBusConnection::systemBus(),
                                          this);

    if (!m_vaultInterface->isValid()) {
        qDebug() << m_vaultInterface->lastError().message();
    }

    // 定时刷新访问时间
    connect(&m_refreshTimer, &QTimer::timeout, this, &VaultLockManager::refreshAccessTime);
    m_refreshTimer.setInterval(1000);

    // 自动锁
    connect(&m_alarmClock, &QTimer::timeout, this, &VaultLockManager::processAutoLock);
    m_alarmClock.setInterval(1000);

    // 用于测试，后面删除
    autoLock(FiveMinutes);
}

VaultLockManager::~VaultLockManager()
{

}

VaultLockManager::AutoLockState VaultLockManager::autoLockState() const
{
    return m_lockState;
}

bool VaultLockManager::autoLock(VaultLockManager::AutoLockState lockState)
{
    m_lockState = lockState;

    if (m_lockState == Never) {
        m_refreshTimer.stop();
        m_alarmClock.stop();
    } else {
        m_refreshTimer.start();
        m_alarmClock.start();
    }
    return true;
}

void VaultLockManager::refreshAccessTime()
{
    if (m_rootFileInfo.data() == nullptr) {
        m_rootFileInfo = DFileService::instance()->createFileInfo(
                    nullptr, VaultController::getVaultController()->makeVaultUrl());
    }
    qint64 time = m_rootFileInfo->lastRead().toSecsSinceEpoch();
    dbusSetRefreshTime(static_cast<quint64>(time));
}

void VaultLockManager::processAutoLock()
{
    VaultController *controller = VaultController::getVaultController();
    if (controller->state() != VaultController::Unlocked)
        return;

    if (m_lockState == Never) {

    } else {
        quint64 lastAccessTime = dbusGetLastestTime();

        QDateTime local(QDateTime::currentDateTime());
        quint64 curTime = static_cast<quint64>(local.toSecsSinceEpoch());

        quint64 interval = curTime - lastAccessTime;
        quint32 threshold = m_lockState * 60;

        if (interval > threshold) {
            controller->lockVault();
        }
    }
}

bool VaultLockManager::isValid() const
{
    return m_vaultInterface->isValid();
}

void VaultLockManager::dbusSetRefreshTime(quint64 time)
{
    if (m_vaultInterface->isValid()) {
        QDBusPendingReply<> reply = m_vaultInterface->setRefreshTime(time);
        reply.waitForFinished();
        if(reply.isError()) {
            qDebug() << reply.error().message();
        }
    }
}

quint64 VaultLockManager::dbusGetLastestTime() const
{
    quint64 latestTime = 0;
    if (m_vaultInterface->isValid()) {
        QDBusPendingReply<quint64> reply = m_vaultInterface->getLastestTime();
        reply.waitForFinished();
        if (reply.isError()) {
            qDebug() << reply.error().message();
        } else {
            latestTime = reply.value();
        }
    }
    return latestTime;
}
