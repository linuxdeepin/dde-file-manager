// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultmanagerdbus.h"
#include "daemonplugin_vaultdaemon_global.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/application/settings.h>

#include <dfm-framework/dpf.h>

#include <QDateTime>
#include <QTimer>
#include <QFileInfo>
#include <QTimerEvent>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>

#include <unistd.h>

static constexpr int kErrorInputTime { 6 };   // 错误次数
static constexpr int kTimerOutTime { 60 * 1000 };   // 计时器超时时间/ms
static constexpr int kTotalWaitTime { 10 };   // 需要等待的分钟数

DFMBASE_USE_NAMESPACE
DAEMONPVAULT_USE_NAMESPACE

VaultManagerDBus::VaultManagerDBus(QObject *parent)
    : QObject(parent)
{
    curVaultClock = new VaultClock(this);
    currentUser = GetCurrentUser();
    mapUserClock.insert(currentUser, curVaultClock);

    UniversalUtils::userChange(this, SLOT(SysUserChanged(QString)));
    UniversalUtils::prepareForSleep(this, SLOT(computerSleep(bool)));
}

void VaultManagerDBus::SysUserChanged(const QString &curUser)
{
    if (currentUser != curUser) {
        currentUser = curUser;
        bool bContain = mapUserClock.contains(currentUser);
        if (bContain) {
            curVaultClock = mapUserClock[currentUser];
        } else {
            curVaultClock = new VaultClock(this);
            mapUserClock.insert(currentUser, curVaultClock);
        }
    }
}

void VaultManagerDBus::SetRefreshTime(quint64 time)
{
    curVaultClock->setRefreshTime(time);
}

quint64 VaultManagerDBus::GetLastestTime() const
{
    return curVaultClock->getLastestTime();
}

quint64 VaultManagerDBus::GetSelfTime() const
{
    return curVaultClock->getSelfTime();
}

bool VaultManagerDBus::IsLockEventTriggered() const
{
    return curVaultClock->isLockEventTriggered();
}

void VaultManagerDBus::TriggerLockEvent()
{
    curVaultClock->triggerLockEvent();
}

void VaultManagerDBus::ClearLockEvent()
{
    curVaultClock->clearLockEvent();
}

void VaultManagerDBus::ComputerSleep(bool bSleep)
{
    if (bSleep) {
        pcTime = QDateTime::currentSecsSinceEpoch();
    } else {
        qint64 diffTime = QDateTime::currentSecsSinceEpoch() - pcTime;
        if (diffTime > 0) {
            for (auto key : mapUserClock.keys()) {
                VaultClock *vaultClock = mapUserClock.value(key);
                if (vaultClock) {
                    vaultClock->addTickTime(diffTime);
                }
            }
        }
    }
}

int VaultManagerDBus::GetLeftoverErrorInputTimes(int userID)
{
    if (!IsValidInvoker())
        return -1;
    if (!mapLeftoverInputTimes.contains(userID))
        mapLeftoverInputTimes[userID] = kErrorInputTime;
    return mapLeftoverInputTimes[userID];
}

void VaultManagerDBus::LeftoverErrorInputTimesMinusOne(int userID)
{
    if (!IsValidInvoker())
        return;
    if (!mapLeftoverInputTimes.contains(userID))
        mapLeftoverInputTimes[userID] = kErrorInputTime;
    --mapLeftoverInputTimes[userID];
}

void VaultManagerDBus::RestoreLeftoverErrorInputTimes(int userID)
{
    if (!IsValidInvoker())
        return;
    mapLeftoverInputTimes[userID] = kErrorInputTime;
}

void VaultManagerDBus::StartTimerOfRestorePasswordInput(int userID)
{
    if (!IsValidInvoker())
        return;
    mapTimer.insert(startTimer(kTimerOutTime), userID);
}

int VaultManagerDBus::GetNeedWaitMinutes(int userID)
{
    if (!IsValidInvoker())
        return 100;
    if (!mapNeedMinutes.contains(userID))
        mapNeedMinutes[userID] = kTotalWaitTime;
    return mapNeedMinutes[userID];
}

void VaultManagerDBus::RestoreNeedWaitMinutes(int userID)
{
    if (!IsValidInvoker())
        return;
    mapNeedMinutes[userID] = kTotalWaitTime;
}

void VaultManagerDBus::timerEvent(QTimerEvent *event)
{
    int timerID = event->timerId();
    if (mapTimer.contains(timerID)) {
        int userID = mapTimer[timerID];
        if (!mapNeedMinutes.contains(userID))
            mapNeedMinutes[userID] = kTotalWaitTime;
        --mapNeedMinutes[userID];
        if (mapNeedMinutes[userID] < 1) {
            killTimer(timerID);
            mapTimer.remove(timerID);
            // 密码剩余输入次数还原，需要等待的分钟数还原
            RestoreLeftoverErrorInputTimes(userID);
            RestoreNeedWaitMinutes(userID);
        }
    }
}

bool VaultManagerDBus::IsValidInvoker()
{
#ifdef QT_DEBUG
    return true;
#else
    static QStringList kVaultWhiteProcess = { "/usr/bin/dde-file-manager", "/usr/libexec/dde-file-manager" };
    if (connection().isConnected()) {
        uint pid = connection().interface()->servicePid(message().service()).value();
        QFileInfo f(QString("/proc/%1/exe").arg(pid));
        if (!f.exists())
            return false;
        QString Path = f.canonicalFilePath();
        return kVaultWhiteProcess.contains(Path);
    }
    fmWarning() << "Failed to get pid. The caller is not a member of the whitelist";
    return false;
#endif
}

QString VaultManagerDBus::GetCurrentUser() const
{
    return UniversalUtils::getCurrentUser();
}
