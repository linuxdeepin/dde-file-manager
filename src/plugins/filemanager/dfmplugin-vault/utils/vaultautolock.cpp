#include "vaultautolock.h"
#include "vaulthelper.h"
#include "vaultdbusutils.h"

#include "dfm-base/utils/universalutils.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"

#include <QDebug>

#include <unistd.h>

DFMBASE_USE_NAMESPACE
DPVAULT_USE_NAMESPACE
constexpr char kVaultGroup[] = { "Vault/AutoLock" };
constexpr char kVaultAutoLockKey[] = { "AutoLock" };
VaultAutoLock::VaultAutoLock(QObject *parent)
    : QObject(parent)
{
    connect(&alarmClock, &QTimer::timeout, this, &VaultAutoLock::processAutoLock);
    alarmClock.setInterval(1000);
    UniversalUtils::lockEventTriggered(this, SLOT(slotLockEvent(QString)));
    loadConfig();
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
    } else {
        if (isCacheTimeReloaded) {
            refreshAccessTime();
        }

        alarmClock.start();
    }
    isCacheTimeReloaded = true;

    Application::genericSetting()->setValue(kVaultGroup, kVaultAutoLockKey, lockState);

    return true;
}

bool VaultAutoLock::isValid() const
{
    bool bValid = false;

    QVariant value = VaultDBusUtils::vaultManagerDBusCall(QString("GetLastestTime"));
    if (!value.isNull()) {
        bValid = true;
    }

    return bValid;
}

void VaultAutoLock::refreshAccessTime()
{
    if (isValid()) {
        quint64 curTime = dbusGetSelfTime();
        dbusSetRefreshTime(static_cast<quint64>(curTime));
    }
}

void VaultAutoLock::resetConfig()
{
    autoLock(kNever);
}

void VaultAutoLock::processAutoLock()
{
    if (VaultHelper::instance()->state(VaultHelper::instance()->vaultLockPath()) != VaultState::kUnlocked
        || autoLockState == kNever) {

        return;
    }

    quint64 lastAccessTime = dbusGetLastestTime();

    quint64 curTime = dbusGetSelfTime();

    quint64 interval = curTime - lastAccessTime;
    quint32 threshold = autoLockState * 60;

#ifdef AUTOLOCK_TEST
    qDebug() << "vault autolock countdown > " << interval;
#endif

    if (interval > threshold) {
        qDebug() << "-----------enter interval > threshold-------------";
        //        VaultAutoLock::killVaultTasks();
        qDebug() << "---------------begin lockVault---------------";
        VaultHelper::instance()->lockVault();
        qDebug() << "---------------leave lockVault---------------";
    }
}

void VaultAutoLock::slotUnlockVault(int state)
{
    if (state == 0) {
        autoLock(autoLockState);
    }
}

void VaultAutoLock::slotLockVault(int state)
{
    if (state == 0) {
        alarmClock.stop();
    } else {
        qDebug() << "vault cannot lock";
    }
}

void VaultAutoLock::processLockEvent()
{
    VaultHelper::instance()->lockVault();
}

void VaultAutoLock::slotLockEvent(const QString &user)
{
    char *loginUser = getlogin();
    if (user == loginUser) {
        processLockEvent();
    }
}

void VaultAutoLock::loadConfig()
{
    AutoLockState state = kNever;
    QVariant var = Application::genericSetting()->value(kVaultGroup, kVaultAutoLockKey);
    if (var.isValid()) {
        state = static_cast<AutoLockState>(var.toInt());
    }
    autoLock(state);
}

void VaultAutoLock::dbusSetRefreshTime(quint64 time)
{
    QVariant value = VaultDBusUtils::vaultManagerDBusCall(QString("SetRefreshTime"), QVariant::fromValue(time));
    if (value.isNull()) {
        qInfo() << "SetRefreshTime failed";
    }
}

quint64 VaultAutoLock::dbusGetLastestTime() const
{
    quint64 latestTime = 0;
    QVariant value = VaultDBusUtils::vaultManagerDBusCall(QString("GetLastestTime"));
    if (!value.isNull()) {
        latestTime = value.toULongLong();
    }
    return latestTime;
}

quint64 VaultAutoLock::dbusGetSelfTime() const
{
    quint64 selfTime = 0;
    QVariant value = VaultDBusUtils::vaultManagerDBusCall(QString("GetSelfTime"));
    if (!value.isNull()) {
        selfTime = value.toULongLong();
    }
    return selfTime;
}

VaultAutoLock *VaultAutoLock::instance()
{
    static VaultAutoLock vaultAutoLock;
    return &vaultAutoLock;
}
