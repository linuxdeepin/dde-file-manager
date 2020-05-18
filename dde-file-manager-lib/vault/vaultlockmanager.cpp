#include "vaultlockmanager.h"
#include "dfileservices.h"
#include "dfmsettings.h"
#include "dfmapplication.h"
#include "controllers/vaultcontroller.h"

#include "../dde-file-manager-daemon/dbusservice/dbusinterface/vault_interface.h"

#define VAULT_AUTOLOCK_KEY      "AutoLock"
#define VAULT_GROUP             "Vault/AutoLock"

VaultLockManager &VaultLockManager::getInstance()
{
    static VaultLockManager instance;
    return instance;
}

VaultLockManager::VaultLockManager(QObject *parent) : QObject(parent)
  , m_autoLockState(VaultLockManager::Never)
  , m_isCacheTimeReloaded(false)
{
    m_vaultInterface = new VaultInterface("com.deepin.filemanager.daemon",
                                          "/com/deepin/filemanager/daemon/VaultManager",
                                          QDBusConnection::systemBus(),
                                          this);

    if (!isValid()) {
        qDebug() << m_vaultInterface->lastError().message();
        return;
    }

    // 自动锁计时处理
    connect(&m_alarmClock, &QTimer::timeout, this, &VaultLockManager::processAutoLock);
    m_alarmClock.setInterval(1000);

    connect(VaultController::getVaultController(), &VaultController::sigLockVault, this,  &VaultLockManager::slotLockVault);
    connect(VaultController::getVaultController(), &VaultController::signalUnlockVault, this,  &VaultLockManager::slotUnlockVault);

    loadConfig();
}

VaultLockManager::~VaultLockManager()
{

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

VaultLockManager::AutoLockState VaultLockManager::autoLockState() const
{
    return m_autoLockState;
}

bool VaultLockManager::autoLock(VaultLockManager::AutoLockState lockState)
{
    m_autoLockState = lockState;

    if (m_autoLockState == Never) {
        m_alarmClock.stop();
    } else {
        if (m_isCacheTimeReloaded) {
            refreshAccessTime();
        }

        m_alarmClock.start();
    }
    m_isCacheTimeReloaded = true;

    DFMApplication::genericSetting()->setValue(VAULT_GROUP, VAULT_AUTOLOCK_KEY, lockState);

    return true;
}

void VaultLockManager::refreshAccessTime()
{
    QDateTime local(QDateTime::currentDateTime());
    quint64 curTime = static_cast<quint64>(local.toSecsSinceEpoch());

    dbusSetRefreshTime(static_cast<quint64>(curTime));
}

void VaultLockManager::processAutoLock()
{
    VaultController *controller = VaultController::getVaultController();
    if (controller->state() != VaultController::Unlocked
            || m_autoLockState == Never) {

        return;
    }

    quint64 lastAccessTime = dbusGetLastestTime();

    QDateTime local(QDateTime::currentDateTime());
    quint64 curTime = static_cast<quint64>(local.toSecsSinceEpoch());

    quint64 interval = curTime - lastAccessTime;
    quint32 threshold = m_autoLockState * 60;

    qDebug() << "vault autolock countdown > " << interval;

    if (interval > threshold) {
        controller->lockVault();
    }
}

void VaultLockManager::slotLockVault(QString msg)
{
    if (msg.contains("vault_unlocked")) {
        m_alarmClock.stop();
    }
}

void VaultLockManager::slotUnlockVault()
{
    autoLock(m_autoLockState);
}

bool VaultLockManager::isValid() const
{
    bool bValid = false;

    if (m_vaultInterface->isValid()) {
        QDBusPendingReply<quint64> reply = m_vaultInterface->getLastestTime();
        reply.waitForFinished();
        bValid = !reply.isError();
    }
    return bValid;
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
