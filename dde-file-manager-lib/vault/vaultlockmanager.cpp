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
  , m_autoLockState(VaultLockManager::Never)
  , m_isCacheTimeReloaded(false)
{
    m_vaultInterface = new VaultInterface("com.deepin.filemanager.daemon",
                                          "/com/deepin/filemanager/daemon/VaultManager",
                                          QDBusConnection::systemBus(),
                                          this);

    if (!m_vaultInterface->isValid()) {
        qDebug() << m_vaultInterface->lastError().message();
    }

    // 自动锁计时处理
    connect(&m_alarmClock, &QTimer::timeout, this, &VaultLockManager::processAutoLock);
    m_alarmClock.setInterval(1000);

    connect(VaultController::getVaultController(), &VaultController::sigLockVault, this,  &VaultLockManager::slotLockVault);
    connect(VaultController::getVaultController(), &VaultController::signalUnlockVault, this,  &VaultLockManager::slotUnlockVault);
}

VaultLockManager::~VaultLockManager()
{

}

void VaultLockManager::loadConfig()
{
    // Somthing to do.
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

    refreshAccessTime();
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
