// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultautolock.h"
#include "vaulthelper.h"
#include "pathmanager.h"
#include "dbus/vaultdbusutils.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include <QDebug>

#include <unistd.h>

constexpr char kVaultGroup[] = { "Vault/AutoLock" };
constexpr char kVaultAutoLockKey[] { "AutoLock" };

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultAutoLock::VaultAutoLock(QObject *parent)
    : QObject(parent)
{
    fmDebug() << "Vault: Initializing VaultAutoLock";

    connect(&alarmClock, &QTimer::timeout, this, &VaultAutoLock::processAutoLock);
    alarmClock.setInterval(1000);
    VaultDBusUtils::lockEventTriggered(this, SLOT(slotLockEvent(QString)));
    loadConfig();
    fmDebug() << "Vault: VaultAutoLock initialization completed";
}

/*!
 * \brief     自动上锁状态
 * \return    返回状态值
 */
VaultAutoLock::AutoLockState VaultAutoLock::getAutoLockState() const
{
    return autoLockState;
}

bool VaultAutoLock::autoLock(VaultAutoLock::AutoLockState lockState)
{
    autoLockState = lockState;

    if (autoLockState == kNever) {
        alarmClock.stop();
        fmDebug() << "Vault: Auto-lock disabled, timer stopped";
    } else {
        if (isCacheTimeReloaded) {
            fmDebug() << "Vault: Refreshing access time for auto-lock";
            refreshAccessTime();
        }

        alarmClock.start();
        fmDebug() << "Vault: Auto-lock timer started with state:" << static_cast<int>(autoLockState);
    }
    isCacheTimeReloaded = true;

    Application::genericSetting()->setValue(kVaultGroup, kVaultAutoLockKey, lockState);

    return true;
}

bool VaultAutoLock::isValid() const
{
    fmDebug() << "Vault: Checking auto-lock validity";

    bool bValid = false;

    QVariant value = VaultDBusUtils::vaultManagerDBusCall(QString("GetLastestTime"));
    if (!value.isNull()) {
        bValid = true;
        fmDebug() << "Vault: Auto-lock is valid, DBus call successful";
    } else {
        fmWarning() << "Vault: Auto-lock is invalid, DBus call failed";
    }

    return bValid;
}

void VaultAutoLock::refreshAccessTime()
{
    fmDebug() << "Vault: Refreshing access time";

    if (isValid()) {
        quint64 curTime = dbusGetSelfTime();
        fmDebug() << "Vault: Current time retrieved:" << curTime;
        dbusSetRefreshTime(static_cast<quint64>(curTime));
    } else {
        fmWarning() << "Vault: Cannot refresh access time - auto-lock is not valid";
    }
}

void VaultAutoLock::resetConfig()
{
    autoLock(kNever);
}

void VaultAutoLock::processAutoLock()
{
    if (VaultHelper::instance()->state(PathManager::vaultLockPath()) != VaultState::kUnlocked
        || autoLockState == kNever) {
        fmDebug() << "Vault: Auto-lock skipped - vault not unlocked";
        return;
    }

    quint64 lastAccessTime = dbusGetLastestTime();
    quint64 curTime = dbusGetSelfTime();

    quint64 interval = curTime - lastAccessTime;
    quint32 threshold = autoLockState * 60;

    fmDebug() << "Vault: Auto-lock check - interval:" << interval << "threshold:" << threshold;

#ifdef AUTOLOCK_TEST
    fmDebug() << "vault autolock countdown > " << interval;
#endif

    if (interval > threshold) {
        fmInfo() << "Vault: Auto-lock threshold exceeded, locking vault";
        //        VaultAutoLock::killVaultTasks();
        if (!VaultHelper::instance()->lockVault(false)) {
            fmWarning() << "Lock vault failed!";
        } else {
            fmInfo() << "Vault: Auto-lock completed successfully";
        }
    }
}

void VaultAutoLock::slotUnlockVault(int state)
{
    fmDebug() << "Vault: Unlock vault slot called with state:" << state;

    if (state == 0) {
        fmInfo() << "Vault: Vault unlocked successfully, restarting auto-lock";
        autoLock(autoLockState);
    } else {
        fmWarning() << "Vault: Vault unlock failed with state:" << state;
    }
}

void VaultAutoLock::slotLockVault(int state)
{
    fmDebug() << "Vault: Lock vault slot called with state:" << state;

    if (state == 0) {
        alarmClock.stop();
        fmInfo() << "Vault: Vault locked successfully, auto-lock timer stopped";
    } else {
        fmCritical() << "Vault: cannot lock!";
    }
}

void VaultAutoLock::processLockEvent()
{
    if (VaultHelper::instance()->lockVault(false))
        fmWarning() << "Lock vault failed!";
}

void VaultAutoLock::slotLockEvent(const QString &user)
{
    fmDebug() << "Vault: Lock event received for user:" << user;

    char *loginUser = getlogin();
    if (user == loginUser) {
        fmInfo() << "Vault: Lock event matches current user, processing lock";
        processLockEvent();
    } else {
        fmDebug() << "Vault: Lock event for different user, ignoring";
    }
}

void VaultAutoLock::loadConfig()
{
    fmDebug() << "Vault: Loading auto-lock configuration";

    AutoLockState state = kNever;
    QVariant var = Application::genericSetting()->value(kVaultGroup, kVaultAutoLockKey);
    if (var.isValid()) {
        state = static_cast<AutoLockState>(var.toInt());
        fmDebug() << "Vault: Loaded auto-lock state from config:" << static_cast<int>(state);
    } else {
        fmDebug() << "Vault: No auto-lock configuration found, using default (Never)";
    }

    autoLock(state);
}

void VaultAutoLock::dbusSetRefreshTime(quint64 time)
{
    fmDebug() << "Vault: Setting refresh time via DBus:" << time;

    QVariant value = VaultDBusUtils::vaultManagerDBusCall(QString("SetRefreshTime"), QVariant::fromValue(time));
    if (value.isNull()) {
        fmCritical() << "Vault: set refresh time failed!";
    } else {
        fmDebug() << "Vault: Refresh time set successfully";
    }
}

quint64 VaultAutoLock::dbusGetLastestTime() const
{
    fmDebug() << "Vault: Getting latest time via DBus";

    quint64 latestTime = 0;
    QVariant value = VaultDBusUtils::vaultManagerDBusCall(QString("GetLastestTime"));
    if (!value.isNull()) {
        latestTime = value.toULongLong();
        fmDebug() << "Vault: Retrieved latest time:" << latestTime;
    } else {
        fmWarning() << "Vault: Failed to get latest time via DBus";
    }
    return latestTime;
}

quint64 VaultAutoLock::dbusGetSelfTime() const
{
    fmDebug() << "Vault: Getting self time via DBus";

    quint64 selfTime = 0;
    QVariant value = VaultDBusUtils::vaultManagerDBusCall(QString("GetSelfTime"));
    if (!value.isNull()) {
        selfTime = value.toULongLong();
        fmDebug() << "Vault: Retrieved self time:" << selfTime;
    } else {
        fmWarning() << "Vault: Failed to get self time via DBus";
    }
    return selfTime;
}

VaultAutoLock *VaultAutoLock::instance()
{
    static VaultAutoLock vaultAutoLock;
    return &vaultAutoLock;
}
