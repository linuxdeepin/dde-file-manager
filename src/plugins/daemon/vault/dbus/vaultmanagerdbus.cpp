// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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

    fmInfo() << "[VaultManagerDBus::VaultManagerDBus] Initialized for user:" << currentUser;

    UniversalUtils::userChange(this, SLOT(SysUserChanged(QString)));
    UniversalUtils::prepareForSleep(this, SLOT(computerSleep(bool)));
}

void VaultManagerDBus::SysUserChanged(const QString &curUser)
{
    if (currentUser != curUser) {
        fmInfo() << "[VaultManagerDBus::SysUserChanged] User changed from:" << currentUser << "to:" << curUser;
        currentUser = curUser;
        bool bContain = mapUserClock.contains(currentUser);
        if (bContain) {
            curVaultClock = mapUserClock[currentUser];
            fmDebug() << "[VaultManagerDBus::SysUserChanged] Using existing clock for user:" << currentUser;
        } else {
            curVaultClock = new VaultClock(this);
            mapUserClock.insert(currentUser, curVaultClock);
            fmDebug() << "[VaultManagerDBus::SysUserChanged] Created new clock for user:" << currentUser;
        }
    }
}

void VaultManagerDBus::SetRefreshTime(quint64 time)
{
    fmDebug() << "[VaultManagerDBus::SetRefreshTime] Setting refresh time:" << time << "for user:" << currentUser;
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
        fmInfo() << "[VaultManagerDBus::ComputerSleep] Computer entering sleep mode, recording time:" << pcTime;
    } else {
        qint64 diffTime = QDateTime::currentSecsSinceEpoch() - pcTime;
        fmInfo() << "[VaultManagerDBus::ComputerSleep] Computer waking up, sleep duration:" << diffTime << "seconds";
        if (diffTime > 0) {
            for (auto key : mapUserClock.keys()) {
                VaultClock *vaultClock = mapUserClock.value(key);
                if (vaultClock) {
                    vaultClock->addTickTime(diffTime);
                }
            }
            fmDebug() << "[VaultManagerDBus::ComputerSleep] Updated tick time for" << mapUserClock.size() << "user clocks";
        }
    }
}

int VaultManagerDBus::GetLeftoverErrorInputTimes(int userID)
{
    if (!IsValidInvoker()) {
        fmWarning() << "[VaultManagerDBus::GetLeftoverErrorInputTimes] Invalid invoker for user ID:" << userID;
        return -1;
    }
    if (!mapLeftoverInputTimes.contains(userID))
        mapLeftoverInputTimes[userID] = kErrorInputTime;
    
    fmDebug() << "[VaultManagerDBus::GetLeftoverErrorInputTimes] Remaining attempts for user" << userID << ":" << mapLeftoverInputTimes[userID];
    return mapLeftoverInputTimes[userID];
}

void VaultManagerDBus::LeftoverErrorInputTimesMinusOne(int userID)
{
    if (!IsValidInvoker()) {
        fmWarning() << "[VaultManagerDBus::LeftoverErrorInputTimesMinusOne] Invalid invoker for user ID:" << userID;
        return;
    }
    if (!mapLeftoverInputTimes.contains(userID))
        mapLeftoverInputTimes[userID] = kErrorInputTime;
    --mapLeftoverInputTimes[userID];
    
    fmInfo() << "[VaultManagerDBus::LeftoverErrorInputTimesMinusOne] Decremented error count for user" << userID << ", remaining:" << mapLeftoverInputTimes[userID];
}

void VaultManagerDBus::RestoreLeftoverErrorInputTimes(int userID)
{
    if (!IsValidInvoker()) {
        fmWarning() << "[VaultManagerDBus::RestoreLeftoverErrorInputTimes] Invalid invoker for user ID:" << userID;
        return;
    }
    restoreLeftoverErrorInputTimes(userID);
}

void VaultManagerDBus::restoreLeftoverErrorInputTimes(int userID)
{
    mapLeftoverInputTimes[userID] = kErrorInputTime;
    fmInfo() << "[VaultManagerDBus::restoreLeftoverErrorInputTimes] Restored error input times for user:" << userID;
}

void VaultManagerDBus::StartTimerOfRestorePasswordInput(int userID)
{
    if (!IsValidInvoker()) {
        fmWarning() << "[VaultManagerDBus::StartTimerOfRestorePasswordInput] Invalid invoker for user ID:" << userID;
        return;
    }
    int timerID = startTimer(kTimerOutTime);
    mapTimer.insert(timerID, userID);
    fmInfo() << "[VaultManagerDBus::StartTimerOfRestorePasswordInput] Started password restore timer for user:" << userID << "timer ID:" << timerID;
}

int VaultManagerDBus::GetNeedWaitMinutes(int userID)
{
    if (!IsValidInvoker()) {
        fmWarning() << "[VaultManagerDBus::GetNeedWaitMinutes] Invalid invoker for user ID:" << userID;
        return 100;
    }
    if (!mapNeedMinutes.contains(userID))
        mapNeedMinutes[userID] = kTotalWaitTime;
    
    fmDebug() << "[VaultManagerDBus::GetNeedWaitMinutes] Wait time for user" << userID << ":" << mapNeedMinutes[userID] << "minutes";
    return mapNeedMinutes[userID];
}

void VaultManagerDBus::RestoreNeedWaitMinutes(int userID)
{
    if (!IsValidInvoker()) {
        fmWarning() << "[VaultManagerDBus::RestoreNeedWaitMinutes] Invalid invoker for user ID:" << userID;
        return;
    }
    restoreNeedWaitMinutes(userID);
}

void VaultManagerDBus::restoreNeedWaitMinutes(int userID)
{
    mapNeedMinutes[userID] = kTotalWaitTime;
    fmInfo() << "[VaultManagerDBus::restoreNeedWaitMinutes] Restored wait time for user:" << userID;
}

void VaultManagerDBus::timerEvent(QTimerEvent *event)
{
    int timerID = event->timerId();
    if (mapTimer.contains(timerID)) {
        int userID = mapTimer[timerID];
        if (!mapNeedMinutes.contains(userID))
            mapNeedMinutes[userID] = kTotalWaitTime;
        --mapNeedMinutes[userID];
        
        fmDebug() << "[VaultManagerDBus::timerEvent] Timer tick for user" << userID << ", remaining minutes:" << mapNeedMinutes[userID];
        
        if (mapNeedMinutes[userID] < 1) {
            killTimer(timerID);
            mapTimer.remove(timerID);
            // 密码剩余输入次数还原,需要等待的分钟数还原,在计时器启动时已经检查权限，这里不再检查
            restoreLeftoverErrorInputTimes(userID);
            restoreNeedWaitMinutes(userID);
            fmInfo() << "[VaultManagerDBus::timerEvent] Timer expired for user" << userID << ", restored input attempts";
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
        if (!f.exists()) {
            fmWarning() << "[VaultManagerDBus::IsValidInvoker] Process info not found for PID:" << pid;
            return false;
        }
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
