#include "vaultmanagerdbus.h"
#include "dfm-base/utils/universalutils.h"

#include <QDateTime>
#include <QTimer>
#include <QFileInfo>
#include <QTimerEvent>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

constexpr int kErrorInputTime { 6 };   // 错误次数
constexpr int kTimerOutTime { 60 * 1000 };   // 计时器超时时间/ms
constexpr int kTotalWaitTime { 10 };   // 需要等待的分钟数

DFMBASE_USE_NAMESPACE

VaultClock::VaultClock(QObject *parent)
    : QObject(parent)
{
    connect(&selfTimer, &QTimer::timeout, this, &VaultClock::Tick);
    selfTimer.setInterval(1000);
    selfTimer.start();
}

VaultClock::~VaultClock()
{
    selfTimer.stop();
}

void VaultClock::SetRefreshTime(quint64 time)
{
    lastestTime = time;
}

quint64 VaultClock::GetLastestTime() const
{
    return lastestTime;
}

quint64 VaultClock::GetSelfTime() const
{
    return selfTime;
}

bool VaultClock::IsLockEventTriggered() const
{
    return isLockEventTriggerd;
}

void VaultClock::TriggerLockEvent()
{
    isLockEventTriggerd = true;
}

void VaultClock::ClearLockEvent()
{
    isLockEventTriggerd = false;
}

void VaultClock::AddTickTime(qint64 seconds)
{
    selfTime += static_cast<quint64>(seconds);
}

void VaultClock::Tick()
{
    selfTime++;
}

VaultManagerDBus::VaultManagerDBus(QObject *parent)
    : QObject(parent)
{
    curVaultClock = new VaultClock(this);
    currentUser = GetCurrentUser();
    mapUserClock.insert(currentUser, curVaultClock);

    UniversalUtils::userChange(this, SLOT(sysUserChanged(QString)));

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
    curVaultClock->SetRefreshTime(time);
}

quint64 VaultManagerDBus::GetLastestTime() const
{
    return curVaultClock->GetLastestTime();
}

quint64 VaultManagerDBus::GetSelfTime() const
{
    return curVaultClock->GetSelfTime();
}

bool VaultManagerDBus::IsLockEventTriggered() const
{
    return curVaultClock->IsLockEventTriggered();
}

void VaultManagerDBus::TriggerLockEvent()
{
    curVaultClock->TriggerLockEvent();
}

void VaultManagerDBus::ClearLockEvent()
{
    curVaultClock->ClearLockEvent();
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
                    vaultClock->AddTickTime(diffTime);
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
    QDBusContext context;
    static QStringList VaultwhiteProcess = { "/usr/bin/dde-file-manager", "/usr/bin/dde-desktop", "/usr/bin/dde-select-dialog-wayland", "/usr/bin/dde-select-dialog-x11" };
    uint pid = context.connection().interface()->servicePid(context.message().service()).value();
    QFileInfo f(QString("/proc/%1/exe").arg(pid));
    if (!f.exists())
        return false;
    return VaultwhiteProcess.contains(f.canonicalFilePath());
}

QString VaultManagerDBus::GetCurrentUser() const
{
    return UniversalUtils::getCurrentUser();
}
