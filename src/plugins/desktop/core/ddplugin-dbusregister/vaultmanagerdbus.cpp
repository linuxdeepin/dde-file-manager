#include "vaultmanagerdbus.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/base/application/settings.h"

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

constexpr int kErrorInputTime { 6 };   // 错误次数
constexpr int kTimerOutTime { 60 * 1000 };   // 计时器超时时间/ms
constexpr int kTotalWaitTime { 10 };   // 需要等待的分钟数

constexpr char kVaultTimeConfigFile[] { "/../dde-file-manager/vaultTimeConfig" };
constexpr char kVaultDecryptDirName[] { "vault_unlocked" };

DFMBASE_USE_NAMESPACE

VaultManagerDBus::VaultManagerDBus(QObject *parent)
    : QObject(parent)
{
    curVaultClock = new VaultClock(this);
    currentUser = GetCurrentUser();
    mapUserClock.insert(currentUser, curVaultClock);

    UniversalUtils::userChange(this, SLOT(SysUserChanged(QString)));

    UniversalUtils::prepareForSleep(this, SLOT(computerSleep(bool)));

    dpfSlotChannel->push("dfmplugin_utils", "slot_VaultHelper_ConnectLockScreenDBus");
    dpfSlotChannel->push("dfmplugin_utils", "slot_VaultHelper_TransparentUnlockVault");

    sessionManagerDBusConnect();
}

void VaultManagerDBus::sessionManagerDBusConnect()
{
    QDBusConnection::sessionBus().connect(
            "com.deepin.SessionManager",
            "/com/deepin/SessionManager",
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged", "sa{sv}as",
            this,
            SLOT(lockScreenVaultLock(QDBusMessage)));
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

void VaultManagerDBus::lockScreenVaultLock(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();
    if (3 != arguments.count())
        return;

    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName != "com.deepin.SessionManager")
        return;

    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    QStringList keys = changedProps.keys();
    for (const QString &prop : keys) {
        if (prop == "Locked") {
            bool bLocked = changedProps[prop].toBool();
            if (bLocked) {
                char *loginUser = getlogin();
                QString user = loginUser ? loginUser : "";
                emit lockEventTriggered(user);

                char buf[512] = { 0 };
                FILE *cmd_pipe = popen("pidof -s dde-file-manager", "r");

                fgets(buf, 512, cmd_pipe);
                pid_t pid = static_cast<pid_t>(strtoul(buf, nullptr, 10));

                if (pid == 0) {
                    QString umountCmd = "fusermount -zu " + VaultClock::vaultBasePath() + "/" + kVaultDecryptDirName;
                    QByteArray umountCmdBytes = umountCmd.toUtf8();
                    system(umountCmdBytes.data());
                    //! 记录保险箱上锁时间
                    Settings setting(kVaultTimeConfigFile);
                    setting.setValue(QString("VaultTime"), QString("LockTime"), QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                }
                pclose(cmd_pipe);
            }
        }
    }
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
    static QStringList kVaultWhiteProcess = { "/usr/bin/dde-file-manager" };
    if (connection().isConnected()) {
        uint pid = connection().interface()->servicePid(message().service()).value();
        QFileInfo f(QString("/proc/%1/exe").arg(pid));
        if (!f.exists())
            return false;
        QString Path = f.canonicalFilePath();
        return kVaultWhiteProcess.contains(Path);
    }
    qWarning() << "Failed to get pid. The caller is not a member of the whitelist";
    return false;
#endif
}

QString VaultManagerDBus::GetCurrentUser() const
{
    return UniversalUtils::getCurrentUser();
}
