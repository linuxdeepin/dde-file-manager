// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskencryptsetup.h"
#include "diskencryptsetup_p.h"
#include "core/cryptsetup.h"
#include "core/dmsetup.h"
#include "workers/cryptworkers.h"
#include "helpers/jobfilehelper.h"
#include "helpers/notificationhelper.h"
#include "helpers/crypttabhelper.h"
#include "helpers/commonhelper.h"
#include "helpers/filesystemhelper.h"
#include "helpers/inhibithelper.h"

#include <dfm-mount/dmount.h>

#include <DConfig>

#include <QDBusMessage>
#include <QtConcurrent>
#include <QDataStream>
#include <QFutureWatcher>
#include <QDir>
#include <QFile>
#include <QProcess>

#include <polkit-qt6-1/PolkitQt1/Authority>

#include <unistd.h>

static constexpr char kActionEncrypt[] { "org.deepin.Filemanager.DiskEncrypt.Encrypt" };
static constexpr char kActionDecrypt[] { "org.deepin.Filemanager.DiskEncrypt.Decrypt" };
static constexpr char kActionChgPwd[] { "org.deepin.Filemanager.DiskEncrypt.ChangePassphrase" };

FILE_ENCRYPT_USE_NS

DiskEncryptSetup::DiskEncryptSetup(QObject *parent)
    : QDBusService(parent),
      QDBusContext(),
      m_dptr(new DiskEncryptSetupPrivate(this))
{
    qInfo() << "[DiskEncryptSetup] Initializing disk encryption service";
    initPolicy(QDBusConnection::SystemBus,
               QString(SERVICE_CONFIG_DIR) + "other/deepin-diskencrypt-service.json");
    dfmmount::DDeviceManager::instance();
    m_dptr->initialize();

    connect(NotificationHelper::instance(), &NotificationHelper::notifyEncryptProgress,
            this, &DiskEncryptSetup::EncryptProgress);
    connect(NotificationHelper::instance(), &NotificationHelper::notifyDecryptProgress,
            this, &DiskEncryptSetup::DecryptProgress);
    qInfo() << "[DiskEncryptSetup] Disk encryption service initialized successfully";
}

bool DiskEncryptSetup::InitEncryption(const QVariantMap &args)
{
    qInfo() << "[DiskEncryptSetup::InitEncryption] Encryption initialization request received";

    if (m_dptr->jobRunning) {
        qWarning() << "[DiskEncryptSetup::InitEncryption] Job already running, cannot create new job";
        return false;
    }

    if (!m_dptr->checkAuth(kActionEncrypt)) {
        qWarning() << "[DiskEncryptSetup::InitEncryption] Authentication failed for encryption action";
        return false;
    }

    if (!m_dptr->validateInitArgs(args)) {
        qCritical() << "[DiskEncryptSetup::InitEncryption] Invalid initialization arguments provided:" << args;
        return false;
    }

    auto type = args.value(disk_encrypt::encrypt_param_keys::kKeyJobType).toString();
    qInfo() << "[DiskEncryptSetup::InitEncryption] Creating encryption worker for job type:" << type;

    auto worker = m_dptr->createInitWorker(type, args);
    if (!worker) {
        qCritical() << "[DiskEncryptSetup::InitEncryption] Failed to create encryption worker for type:" << type;
        return false;
    }

    m_dptr->initThreadConnection(worker);
    connect(worker, &QThread::finished,
            m_dptr, &DiskEncryptSetupPrivate::onInitEncryptFinished);
    worker->start();
    qInfo() << "[DiskEncryptSetup::InitEncryption] Encryption worker started successfully for type:" << type;
    return true;
}

bool DiskEncryptSetup::ResumeEncryption(const QVariantMap &args)
{
    qInfo() << "[DiskEncryptSetup::ResumeEncryption] Encryption resume request received";

    if (m_dptr->jobRunning) {
        qWarning() << "[DiskEncryptSetup::ResumeEncryption] Job already running, cannot resume encryption";
        return false;
    }

    if (!m_dptr->validateResumeArgs(args)) {
        qCritical() << "[DiskEncryptSetup::ResumeEncryption] Invalid resume arguments provided:" << args;
        return false;
    }

    qInfo() << "[DiskEncryptSetup::ResumeEncryption] Resuming encryption with validated arguments";
    m_dptr->resumeEncryption(args);
    return true;
}

bool DiskEncryptSetup::Decryption(const QDBusUnixFileDescriptor &credentialsFd)
{
    qInfo() << "[DiskEncryptSetup::Decryption] Decryption request received via fd";

    if (m_dptr->jobRunning) {
        qWarning() << "[DiskEncryptSetup::Decryption] Job already running, cannot start decryption";
        return false;
    }

    if (!m_dptr->checkAuth(kActionDecrypt)) {
        qWarning() << "[DiskEncryptSetup::Decryption] Authentication failed for decryption action";
        return false;
    }

    // Parse credentials from fd using common method
    QVariantMap args;
    if (!m_dptr->parseCredentialsFromFd(credentialsFd, &args)) {
        qCritical() << "[DiskEncryptSetup::Decryption] Failed to parse credentials from fd";
        return false;
    }

    if (!m_dptr->validateDecryptArgs(args)) {
        qCritical() << "[DiskEncryptSetup::Decryption] Invalid decryption arguments provided:" << args;
        return false;
    }

    auto type = args.value(disk_encrypt::encrypt_param_keys::kKeyJobType).toString();
    qInfo() << "[DiskEncryptSetup::Decryption] Creating decryption worker for job type:" << type;

    auto worker = m_dptr->createDecryptWorker(type, args);
    if (!worker) {
        qCritical() << "[DiskEncryptSetup::Decryption] Failed to create decryption worker for type:" << type;
        return false;
    }

    m_dptr->initThreadConnection(worker);
    connect(worker, &QThread::finished,
            m_dptr, &DiskEncryptSetupPrivate::onDecryptFinished);
    worker->start();
    qInfo() << "[DiskEncryptSetup::Decryption] Decryption worker started successfully for type:" << type;
    return true;
}

bool DiskEncryptSetup::ChangePassphrase(const QDBusUnixFileDescriptor &credentialsFd)
{
    qInfo() << "[DiskEncryptSetup::ChangePassphrase] Passphrase change request received via fd";

    if (!m_dptr->checkAuth(kActionChgPwd)) {
        qWarning() << "[DiskEncryptSetup::ChangePassphrase] Authentication failed for passphrase change action";
        return false;
    }

    // Parse credentials from fd using common method
    QVariantMap args;
    if (!m_dptr->parseCredentialsFromFd(credentialsFd, &args)) {
        qCritical() << "[DiskEncryptSetup::ChangePassphrase] Failed to parse credentials from fd";
        return false;
    }

    if (!m_dptr->validateChgPwdArgs(args)) {
        qCritical() << "[DiskEncryptSetup::ChangePassphrase] Invalid passphrase change arguments provided:" << args;
        return false;
    }

    qInfo() << "[DiskEncryptSetup::ChangePassphrase] Creating passphrase change worker";
    auto worker = new PassphraseChangeWorker(args);
    connect(worker, &QThread::finished,
            m_dptr, &DiskEncryptSetupPrivate::onPassphraseChanged);
    worker->start();
    qInfo() << "[DiskEncryptSetup::ChangePassphrase] Passphrase change worker started successfully";
    return true;
}

void DiskEncryptSetup::SetupAuthArgs(const QDBusUnixFileDescriptor &credentialsFd)
{
    qInfo() << "[DiskEncryptSetup::SetupAuthArgs] Setting up authentication arguments via fd";

    // Parse credentials from fd using common method
    QVariantMap args;
    if (!m_dptr->parseCredentialsFromFd(credentialsFd, &args)) {
        qCritical() << "[DiskEncryptSetup::SetupAuthArgs] Failed to parse credentials from fd";
        return;
    }

    qInfo() << "[DiskEncryptSetup::SetupAuthArgs] Successfully parsed authentication arguments from fd";
    Q_EMIT NotificationHelper::instance()->replyAuthArgs(args);
}

void DiskEncryptSetup::IgnoreAuthSetup()
{
    qInfo() << "[DiskEncryptSetup::IgnoreAuthSetup] Ignoring authentication setup";
    Q_EMIT NotificationHelper::instance()->ignoreAuthSetup();
}

QString DiskEncryptSetup::TpmToken(const QString &dev)
{
    qInfo() << "[DiskEncryptSetup::TpmToken] Retrieving TPM token for device:" << dev;

    QString token;
    crypt_setup_helper::getToken(dev, &token);
    if (token.isEmpty()) {
        qInfo() << "[DiskEncryptSetup::TpmToken] No token found for device, checking holder device:" << dev;
        QString phyDev = dm_setup_helper::findHolderDev(dev);
        if (!phyDev.isEmpty()) {
            crypt_setup_helper::getToken(phyDev, &token);
            qInfo() << "[DiskEncryptSetup::TpmToken] Checked holder device:" << phyDev << "token found:" << !token.isEmpty();
        }
    } else {
        qInfo() << "[DiskEncryptSetup::TpmToken] TPM token found for device:" << dev;
    }

    return token;
}

int DiskEncryptSetup::DeviceStatus(const QString &dev)
{
    qInfo() << "[DiskEncryptSetup::DeviceStatus] Checking encryption status for device:" << dev;

    // check status of device itself.
    auto status = crypt_setup_helper::encryptStatus(dev);
    if (status != disk_encrypt::kStatusNotEncrypted) {
        qInfo() << "[DiskEncryptSetup::DeviceStatus] Device encryption status:" << status << "for device:" << dev;
        return status;
    }

    auto phyDev = dm_setup_helper::findHolderDev(dev);
    return crypt_setup_helper::encryptStatus(phyDev);

    // // check the if device in holder chain is encrypted.
    // auto partitions = dm_setup_helper::procPartitions();
    // QList<dm_setup_helper::ProcPartition> holderChain;
    // for (auto part : partitions) {
    // }

    // return disk_encrypt::kStatusNotEncrypted;
}

QString DiskEncryptSetup::HolderDevice(const QString &dev)
{
    return dm_setup_helper::findHolderDev(dev);
}

bool DiskEncryptSetup::IsTaskEmpty()
{
    return !job_file_helper::hasJobFile();
}

bool DiskEncryptSetup::IsTaskRunning()
{
    return m_dptr->jobRunning;
}

QString DiskEncryptSetup::PendingDecryptionDevice()
{
    qInfo() << "[DiskEncryptSetup::PendingDecryptionDevice] Checking for pending decryption devices";

    QDir d(kUSecBootRoot);
    auto files = d.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (auto f : files) {
        auto name = m_dptr->resolveDeviceByDetachHeaderName(f);
        if (!name.isEmpty()) {
            qInfo() << "[DiskEncryptSetup::PendingDecryptionDevice] Found pending decryption device:" << name << "from header file:" << f;
            return name;
        }
    }
    qInfo() << "[DiskEncryptSetup::PendingDecryptionDevice] No pending decryption devices found";
    return "";
}

DiskEncryptSetupPrivate::DiskEncryptSetupPrivate(DiskEncryptSetup *parent)
    : QObject(parent),
      qptr(parent)
{
    qInfo() << "[DiskEncryptSetupPrivate] Initializing private implementation";

    // Initialize DConfig
    config = Dtk::Core::DConfig::create("org.deepin.dde.file-manager",
                                        "org.deepin.dde.file-manager.diskencrypt",
                                        QString(), this);
    if (!config) {
        qWarning() << "[DiskEncryptSetupPrivate] Failed to create DConfig object";
    } else if (!config->isValid()) {
        qWarning() << "[DiskEncryptSetupPrivate] DConfig object is not valid";
        config->deleteLater();
        config = nullptr;
    } else {
        qInfo() << "[DiskEncryptSetupPrivate] DConfig initialized successfully";
    }

    qptr->lockTimer(true);
}

void DiskEncryptSetupPrivate::initialize()
{
    qInfo() << "[DiskEncryptSetupPrivate::initialize] Starting initialization process";
    auto future = QtConcurrent::run([] {
        filesystem_helper::remountBoot();
        common_helper::createDFMDesktopEntry();
        crypttab_helper::mergeCryptTab();
    });
    Q_UNUSED(future);
    job_file_helper::checkJobs();
    setupConfigWatcher();

    // 同步 DConfig 配置与标志文件状态
    syncConfigWithFileSystem();

    resumeEncryption();
}

void DiskEncryptSetupPrivate::setupConfigWatcher()
{
    if (!config) {
        qWarning() << "[DiskEncryptSetupPrivate::setupConfigWatcher] DConfig is null, cannot setup watcher";
        return;
    }

    connect(config, &Dtk::Core::DConfig::valueChanged,
            this, &DiskEncryptSetupPrivate::onConfigValueChanged);
    qInfo() << "[DiskEncryptSetupPrivate::setupConfigWatcher] Config watcher setup complete";
}

void DiskEncryptSetupPrivate::resumeEncryption(const QVariantMap &args)
{
    auto worker = new ResumeEncryptWorker(args);
    initThreadConnection(worker);
    connect(worker, &ResumeEncryptWorker::requestAuthInfo,
            qptr, &DiskEncryptSetup::WaitAuthInput);
    connect(worker, &QThread::finished,
            this, &DiskEncryptSetupPrivate::onResumeEncryptFinished);
    worker->start();
}

bool DiskEncryptSetupPrivate::checkAuth(const QString &action)
{
    using namespace PolkitQt1;

    QString appBusName = qptr->message().service();
    if (appBusName.isEmpty())
        return false;

    return Authority::Yes == Authority::instance()->checkAuthorizationSync(action, SystemBusNameSubject(appBusName), Authority::AllowUserInteraction);
}

bool DiskEncryptSetupPrivate::validateInitArgs(const QVariantMap &args)
{
    if (args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString().isEmpty())
        return false;
    if (args.value(disk_encrypt::encrypt_param_keys::kKeyDeviceName).toString().isEmpty())
        return false;
    auto type = args.value(disk_encrypt::encrypt_param_keys::kKeyJobType).toString();
    if (!job_file_helper::validJobTypes().contains(type))
        return false;
    return true;
}

bool DiskEncryptSetupPrivate::validateResumeArgs(const QVariantMap &args)
{
    return true;
}

bool DiskEncryptSetupPrivate::validateDecryptArgs(const QVariantMap &args)
{
    if (args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString().isEmpty())
        return false;

    auto type = args.value(disk_encrypt::encrypt_param_keys::kKeyJobType).toString();
    if (!job_file_helper::validJobTypes().contains(type))
        return false;

    if (type != disk_encrypt::job_type::TypeFstab) {
        if (args.value(disk_encrypt::encrypt_param_keys::kKeyPassphrase).toString().isEmpty())
            return false;
        if (args.value(disk_encrypt::encrypt_param_keys::kKeyDeviceName).toString().isEmpty())
            return false;
    }

    return true;
}

bool DiskEncryptSetupPrivate::validateChgPwdArgs(const QVariantMap &args)
{
    if (args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString().isEmpty())
        return false;
    if (args.value(disk_encrypt::encrypt_param_keys::kKeyPassphrase).toString().isEmpty())
        return false;
    if (args.value(disk_encrypt::encrypt_param_keys::kKeyOldPassphrase).toString().isEmpty())
        return false;
    return true;
}

QString DiskEncryptSetupPrivate::resolveDeviceByDetachHeaderName(const QString &fileName)
{
    if (!fileName.startsWith(kUSecDetachHeaderPrefix))
        return "";

    auto puuid = QString(fileName).remove(kUSecDetachHeaderPrefix).remove(".bin");
    auto objPath = blockdev_helper::resolveDevObjPath("PARTUUID=" + puuid);
    if (objPath.isEmpty())
        objPath = blockdev_helper::resolveDevObjPath("/dev/" + puuid);
    if (objPath.isEmpty()) {
        qWarning() << "cannot find device object path!" << fileName;
        return "";
    }

    auto dev = blockdev_helper::createDevPtr2(objPath);
    if (!dev) {
        qWarning() << "cannot create device object!" << objPath;
        return "";
    }

    return dev->device();
}

BaseEncryptWorker *DiskEncryptSetupPrivate::createInitWorker(const QString &type, const QVariantMap &args)
{
    if (type == disk_encrypt::job_type::TypeFstab)
        return new FstabInitEncryptWorker(args);
    else if (type == disk_encrypt::job_type::TypeOverlay)
        return new DMInitEncryptWorker(args);
    else if (type == disk_encrypt::job_type::TypeNormal)
        return new NormalInitEncryptWorker(args);
    return nullptr;
}

BaseEncryptWorker *DiskEncryptSetupPrivate::createDecryptWorker(const QString &type, const QVariantMap &args)
{
    if (type == disk_encrypt::job_type::TypeFstab)
        return new FstabDecryptWorker(args);
    else if (type == disk_encrypt::job_type::TypeOverlay)
        return new DMDecryptWorker(args);
    else if (type == disk_encrypt::job_type::TypeNormal)
        return new NormalDecryptWorker(args);
    return nullptr;
}

bool DiskEncryptSetupPrivate::parseCredentialsFromFd(const QDBusUnixFileDescriptor &credentialsFd, QVariantMap *args)
{
    Q_ASSERT(args);
    // Validate file descriptor
    if (!credentialsFd.isValid()) {
        qWarning() << "[DiskEncryptSetupPrivate::parseCredentialsFromFd] Invalid file descriptor provided";
        return false;
    }

    int fd = credentialsFd.fileDescriptor();
    if (fd < 0) {
        qWarning() << "[DiskEncryptSetupPrivate::parseCredentialsFromFd] Invalid file descriptor value:" << fd;
        return false;
    }

    // Read all data from pipe into buffer
    QByteArray buffer;
    char readBuffer[1024];
    ssize_t bytesRead;

    while ((bytesRead = read(fd, readBuffer, sizeof(readBuffer))) > 0) {
        buffer.append(readBuffer, bytesRead);
    }

    if (buffer.isEmpty()) {
        qWarning() << "[DiskEncryptSetupPrivate::parseCredentialsFromFd] No data received from pipe";
        return false;
    }

    // Parse credentials using QDataStream
    QDataStream stream(&buffer, QIODevice::ReadOnly);
    stream >> *args;

    if (stream.status() != QDataStream::Ok) {
        qWarning() << "[DiskEncryptSetupPrivate::parseCredentialsFromFd] Failed to parse credentials from pipe data, stream status:" << stream.status();
        return false;
    }

    qInfo() << "[DiskEncryptSetupPrivate::parseCredentialsFromFd] Successfully parsed credentials from fd";
    return true;
}

void DiskEncryptSetupPrivate::initThreadConnection(const QThread *thread)
{
    connect(thread, &QThread::started, this, &DiskEncryptSetupPrivate::onLongTimeJobStarted);
    connect(thread, &QThread::finished, this, &DiskEncryptSetupPrivate::onLongTimeJobStopped);
}

void DiskEncryptSetupPrivate::onInitEncryptFinished()
{
    auto worker = dynamic_cast<BaseEncryptWorker *>(sender());
    if (!worker) return;
    worker->deleteLater();

    auto args = worker->args();
    auto code = worker->exitCode();
    qInfo() << "device encryption initialized." << code << args;
    if (code == disk_encrypt::kSuccess) {
        // system("udevadm trigger");
        resumeEncryption();
    }

    using namespace disk_encrypt::encrypt_param_keys;
    auto result = QVariantMap { { kKeyDevice, args.value(kKeyDevice).toString() },
                                { kKeyDeviceName, args.value(kKeyDeviceName).toString() },
                                { kKeyOperationResult, code } };
    Q_EMIT qptr->InitEncResult(result);
}

void DiskEncryptSetupPrivate::onResumeEncryptFinished()
{
    auto worker = dynamic_cast<ResumeEncryptWorker *>(sender());
    if (!worker) return;
    worker->deleteLater();

    auto code = worker->exitCode();
    auto args = worker->args();

    if (code == -disk_encrypt::kIgnoreRequest)
        return;

    using namespace disk_encrypt::encrypt_param_keys;
    if (args.value(kKeyDevice).toString().isEmpty())
        return;
    auto result = QVariantMap { { kKeyDevice, args.value(kKeyDevice).toString() },
                                { kKeyDeviceName, args.value(kKeyDeviceName).toString() },
                                { kKeyOperationResult, code } };

    // save failed, ask front to save it again.
    if (code == -disk_encrypt::KErrorRequestExportRecKey)
        result.insert(kKeyRecoveryKey, worker->recoveryKey());

    Q_EMIT qptr->EncryptResult(result);
}

void DiskEncryptSetupPrivate::onDecryptFinished()
{
    auto worker = dynamic_cast<BaseEncryptWorker *>(sender());
    if (!worker) return;
    worker->deleteLater();

    auto code = worker->exitCode();
    auto args = worker->args();

    using namespace disk_encrypt::encrypt_param_keys;
    auto result = QVariantMap { { kKeyDevice, args.value(kKeyDevice).toString() },
                                { kKeyDeviceName, args.value(kKeyDeviceName).toString() },
                                { kKeyOperationResult, code } };
    Q_EMIT qptr->DecryptResult(result);
}

void DiskEncryptSetupPrivate::onPassphraseChanged()
{
    auto worker = dynamic_cast<BaseEncryptWorker *>(sender());
    if (!worker) return;
    worker->deleteLater();

    auto code = worker->exitCode();
    auto args = worker->args();
    using namespace disk_encrypt::encrypt_param_keys;
    auto result = QVariantMap { { kKeyDevice, args.value(kKeyDevice).toString() },
                                { kKeyDeviceName, args.value(kKeyDeviceName).toString() },
                                { kKeyOperationResult, code } };
    Q_EMIT qptr->ChangePassResult(result);
}

void DiskEncryptSetupPrivate::onLongTimeJobStarted()
{
    jobRunning = true;
    // qptr->lockTimer(true);
    // qInfo() << "auto quit timer is locked.";
}

void DiskEncryptSetupPrivate::onLongTimeJobStopped()
{
    jobRunning = false;
    // qptr->lockTimer(false);
    // qInfo() << "auto quite timer is unlocked";
}

void DiskEncryptSetupPrivate::onConfigValueChanged(const QString &key)
{
    qInfo() << "[DiskEncryptSetupPrivate::onConfigValueChanged] Config value changed:" << key;

    if (key == "useOverlayDMMode") {
        if (!config) {
            qWarning() << "[DiskEncryptSetupPrivate::onConfigValueChanged] DConfig is null";
            return;
        }

        bool newValue = config->value("useOverlayDMMode", false).toBool();
        qInfo() << "[DiskEncryptSetupPrivate::onConfigValueChanged] useOverlayDMMode changed to:" << newValue;

        // Check if already handling a config change
        if (isHandlingConfigChange) {
            // 正在处理配置变更
            if (newValue == currentTargetValue) {
                // 新值等于正在执行的目标值，可以忽略
                qInfo() << "[DiskEncryptSetupPrivate::onConfigValueChanged] New value equals current target value, ignoring";
                hasPendingConfigChange = false;   // 清除待处理标记
                return;
            } else {
                // 新值不同于正在执行的目标值，记录为待处理
                qWarning() << "[DiskEncryptSetupPrivate::onConfigValueChanged] Already handling config change to"
                           << currentTargetValue << ", queueing new target:" << newValue;
                hasPendingConfigChange = true;
                pendingTargetValue = newValue;
                return;
            }
        }

        // 没有正在执行的操作，直接开始异步处理
        currentTargetValue = newValue;
        handleOverlayDMModeChangeAsync(newValue);
    }
}

bool DiskEncryptSetupPrivate::handleOverlayDMModeChange(bool enabled)
{
    qInfo() << "[DiskEncryptSetupPrivate::handleOverlayDMModeChange] Handling mode change, enabled:" << enabled;

    if (enabled) {
        // Create flag file
        if (!createOverlayDMFlagFile()) {
            qCritical() << "[DiskEncryptSetupPrivate::handleOverlayDMModeChange] Failed to create overlay DM flag file";
            return false;
        }

        // Update initramfs
        if (!updateInitramfs()) {
            qCritical() << "[DiskEncryptSetupPrivate::handleOverlayDMModeChange] Failed to update initramfs, rolling back";
            // Rollback: remove the flag file
            if (!removeOverlayDMFlagFile()) {
                qCritical() << "[DiskEncryptSetupPrivate::handleOverlayDMModeChange] Failed to rollback flag file removal";
            }

            // Rollback config value
            if (config) {
                config->setValue("useOverlayDMMode", false);
                qInfo() << "[DiskEncryptSetupPrivate::handleOverlayDMModeChange] Config value rolled back to false";
            }
            return false;
        }

        qInfo() << "[DiskEncryptSetupPrivate::handleOverlayDMModeChange] Overlay DM mode enabled successfully";
        return true;
    } else {
        // Disable mode: remove flag file
        if (!removeOverlayDMFlagFile()) {
            qCritical() << "[DiskEncryptSetupPrivate::handleOverlayDMModeChange] Failed to remove overlay DM flag file";
            return false;
        }

        // Update initramfs to sync the change
        if (!updateInitramfs()) {
            qCritical() << "[DiskEncryptSetupPrivate::handleOverlayDMModeChange] Failed to update initramfs, rolling back";
            // Rollback: recreate the flag file
            if (!createOverlayDMFlagFile()) {
                qCritical() << "[DiskEncryptSetupPrivate::handleOverlayDMModeChange] Failed to rollback flag file creation";
            }

            // Rollback config value
            if (config) {
                config->setValue("useOverlayDMMode", true);
                qInfo() << "[DiskEncryptSetupPrivate::handleOverlayDMModeChange] Config value rolled back to true";
            }
            return false;
        }

        qInfo() << "[DiskEncryptSetupPrivate::handleOverlayDMModeChange] Overlay DM mode disabled successfully";
        return true;
    }
}

bool DiskEncryptSetupPrivate::createOverlayDMFlagFile()
{
    static constexpr char kSettingsDir[] { "/etc/usec-crypt/settings" };
    static constexpr char kFlagFile[] { "/etc/usec-crypt/settings/overlay-dm" };

    qInfo() << "[DiskEncryptSetupPrivate::createOverlayDMFlagFile] Creating overlay DM flag file";

    // Create directory if not exists
    QDir dir(kSettingsDir);
    if (!dir.exists()) {
        if (!dir.mkpath(kSettingsDir)) {
            qCritical() << "[DiskEncryptSetupPrivate::createOverlayDMFlagFile] Failed to create settings directory:" << kSettingsDir;
            return false;
        }
        qInfo() << "[DiskEncryptSetupPrivate::createOverlayDMFlagFile] Settings directory created:" << kSettingsDir;
    }

    // Create empty flag file
    QFile file(kFlagFile);
    if (file.exists()) {
        qInfo() << "[DiskEncryptSetupPrivate::createOverlayDMFlagFile] Flag file already exists:" << kFlagFile;
        return true;
    }

    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "[DiskEncryptSetupPrivate::createOverlayDMFlagFile] Failed to create flag file:" << kFlagFile
                    << "Error:" << file.errorString();
        return false;
    }

    file.close();
    qInfo() << "[DiskEncryptSetupPrivate::createOverlayDMFlagFile] Flag file created successfully:" << kFlagFile;
    return true;
}

bool DiskEncryptSetupPrivate::removeOverlayDMFlagFile()
{
    static constexpr char kFlagFile[] { "/etc/usec-crypt/settings/overlay-dm" };

    qInfo() << "[DiskEncryptSetupPrivate::removeOverlayDMFlagFile] Removing overlay DM flag file";

    QFile file(kFlagFile);
    if (!file.exists()) {
        qInfo() << "[DiskEncryptSetupPrivate::removeOverlayDMFlagFile] Flag file does not exist:" << kFlagFile;
        return true;
    }

    if (!file.remove()) {
        qCritical() << "[DiskEncryptSetupPrivate::removeOverlayDMFlagFile] Failed to remove flag file:" << kFlagFile
                    << "Error:" << file.errorString();
        return false;
    }

    qInfo() << "[DiskEncryptSetupPrivate::removeOverlayDMFlagFile] Flag file removed successfully:" << kFlagFile;
    return true;
}

bool DiskEncryptSetupPrivate::updateInitramfs()
{
    static constexpr char kSudoCmd[] { "/usr/bin/sudo" };
    static constexpr char kUpdateInitramfsCmd[] { "/sbin/update-initramfs" };

    qInfo() << "[DiskEncryptSetupPrivate::updateInitramfs] Executing update-initramfs command via sudo -E";

    // Create inhibit lock to prevent shutdown/sleep during update-initramfs
    QString inhibitMessage = "Updating initramfs, please do not shutdown or suspend the system";
    auto inhibitReply = inhibit_helper::inhibit(inhibitMessage);

    QDBusUnixFileDescriptor inhibitFd;
    bool hasInhibitLock = false;

    if (inhibitReply.isValid()) {
        inhibitFd = inhibitReply.value();
        hasInhibitLock = true;
        qInfo() << "[DiskEncryptSetupPrivate::updateInitramfs] Inhibit lock acquired, fd:" << inhibitFd.fileDescriptor();
    } else {
        qWarning() << "[DiskEncryptSetupPrivate::updateInitramfs] Failed to acquire inhibit lock:" << inhibitReply.error().message();
        qWarning() << "[DiskEncryptSetupPrivate::updateInitramfs] Continuing without inhibit lock (user may be able to shutdown)";
    }

    QProcess process;
    process.start(kSudoCmd, QStringList() << "-E" << kUpdateInitramfsCmd << "-u");

    if (!process.waitForStarted()) {
        QString errorMsg = process.errorString();
        qCritical() << "[DiskEncryptSetupPrivate::updateInitramfs] Failed to start update-initramfs process"
                    << "Error:" << errorMsg << "Error code:" << process.error();
        // Inhibit lock will be released when inhibitFd goes out of scope
        return false;
    }

    // Wait for process to finish (max 5 minutes)
    if (!process.waitForFinished(300000)) {
        qCritical() << "[DiskEncryptSetupPrivate::updateInitramfs] update-initramfs process timed out or failed to finish";
        process.kill();
        // Inhibit lock will be released when inhibitFd goes out of scope
        return false;
    }

    int exitCode = process.exitCode();
    QString stdOut = QString::fromLocal8Bit(process.readAllStandardOutput());
    QString stdErr = QString::fromLocal8Bit(process.readAllStandardError());

    qInfo() << "[DiskEncryptSetupPrivate::updateInitramfs] update-initramfs exit code:" << exitCode;
    if (!stdOut.isEmpty()) {
        qInfo() << "[DiskEncryptSetupPrivate::updateInitramfs] stdout:" << stdOut;
    }
    if (!stdErr.isEmpty()) {
        qWarning() << "[DiskEncryptSetupPrivate::updateInitramfs] stderr:" << stdErr;
    }

    if (exitCode != 0) {
        qCritical() << "[DiskEncryptSetupPrivate::updateInitramfs] update-initramfs failed with exit code:" << exitCode;
        // Inhibit lock will be released when inhibitFd goes out of scope
        return false;
    }

    qInfo() << "[DiskEncryptSetupPrivate::updateInitramfs] update-initramfs completed successfully";

    // Inhibit lock will be automatically released when inhibitFd goes out of scope
    if (hasInhibitLock) {
        qInfo() << "[DiskEncryptSetupPrivate::updateInitramfs] Releasing inhibit lock";
    }

    return true;
}

void DiskEncryptSetupPrivate::syncConfigWithFileSystem()
{
    static constexpr char kFlagFile[] { "/etc/usec-crypt/settings/overlay-dm" };

    if (!config) {
        qWarning() << "[DiskEncryptSetupPrivate::syncConfigWithFileSystem] DConfig is null, cannot sync";
        return;
    }

    // 读取 DConfig 配置值
    bool configValue = config->value("useOverlayDMMode", false).toBool();

    // 检查标志文件是否存在
    bool flagFileExists = QFile::exists(kFlagFile);

    qInfo() << "[DiskEncryptSetupPrivate::syncConfigWithFileSystem] DConfig value:" << configValue
            << "Flag file exists:" << flagFileExists;

    // 更新 currentTargetValue 以反映实际文件系统状态
    currentTargetValue = flagFileExists;

    // 如果 DConfig 和文件系统状态不一致，需要同步
    if (configValue != flagFileExists) {
        qWarning() << "[DiskEncryptSetupPrivate::syncConfigWithFileSystem] State mismatch detected!"
                   << "DConfig says:" << configValue << "but file system is:" << flagFileExists;
        qInfo() << "[DiskEncryptSetupPrivate::syncConfigWithFileSystem] Syncing to match DConfig value:" << configValue;

        // 异步执行同步操作（以 DConfig 为准）
        currentTargetValue = configValue;
        handleOverlayDMModeChangeAsync(configValue);
    } else {
        qInfo() << "[DiskEncryptSetupPrivate::syncConfigWithFileSystem] DConfig and file system are in sync, no action needed";
    }
}

void DiskEncryptSetupPrivate::handleOverlayDMModeChangeAsync(bool enabled)
{
    qInfo() << "[DiskEncryptSetupPrivate::handleOverlayDMModeChangeAsync] Starting async mode change to:" << enabled;

    isHandlingConfigChange = true;

    // Use QtConcurrent to run in background thread
    auto future = QtConcurrent::run([this, enabled]() -> bool {
        return this->handleOverlayDMModeChange(enabled);
    });

    // Watch the future and call callback when done
    auto watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, enabled]() {
        bool success = watcher->result();
        watcher->deleteLater();
        this->onOverlayDMModeChangeFinished(success, enabled);
    });
    watcher->setFuture(future);
}

void DiskEncryptSetupPrivate::onOverlayDMModeChangeFinished(bool success, bool targetValue)
{
    qInfo() << "[DiskEncryptSetupPrivate::onOverlayDMModeChangeFinished] Mode change finished, success:" << success
            << "target value:" << targetValue;

    isHandlingConfigChange = false;

    // Determine result code
    int resultCode = success ? OverlayDMSuccess : OverlayDMFailedUpdateInitramfs;

    if (!success) {
        qCritical() << "[DiskEncryptSetupPrivate::onOverlayDMModeChangeFinished] Mode change failed for target:" << targetValue;

        // Config value already rolled back in handleOverlayDMModeChange
        // Reset currentTargetValue to match the rolled back config
        if (config) {
            currentTargetValue = config->value("useOverlayDMMode", false).toBool();
            qInfo() << "[DiskEncryptSetupPrivate::onOverlayDMModeChangeFinished] Reset currentTargetValue to:" << currentTargetValue;
        }
    } else {
        qInfo() << "[DiskEncryptSetupPrivate::onOverlayDMModeChangeFinished] Mode change succeeded for target:" << targetValue;

        // Update currentTargetValue to ensure consistency
        currentTargetValue = targetValue;
    }

    // Emit DBus signal to notify upper layer application
    qInfo() << "[DiskEncryptSetupPrivate::onOverlayDMModeChangeFinished] Emitting DBus signal, enabled:" << targetValue
            << "result code:" << resultCode;
    Q_EMIT qptr->OverlayDMModeChanged(targetValue, resultCode);

    // Check if there's a pending config change
    processPendingConfigChange();
}

void DiskEncryptSetupPrivate::processPendingConfigChange()
{
    if (!hasPendingConfigChange) {
        qInfo() << "[DiskEncryptSetupPrivate::processPendingConfigChange] No pending config change";
        return;
    }

    qInfo() << "[DiskEncryptSetupPrivate::processPendingConfigChange] Processing pending config change to:" << pendingTargetValue;

    // 与 currentTargetValue（实际文件系统状态）比较，而不是与 DConfig 值比较
    // 因为 DConfig 值在用户修改时会立即变化，
    // 但 currentTargetValue 反映的是已完成操作的实际结果
    if (pendingTargetValue == currentTargetValue) {
        // 待处理的目标值和当前实际状态相同，无需执行
        qInfo() << "[DiskEncryptSetupPrivate::processPendingConfigChange] Pending value" << pendingTargetValue
                << "equals current actual state" << currentTargetValue << ", no action needed";
        hasPendingConfigChange = false;
    } else {
        // 待处理的目标值和当前实际状态不同，需要执行变更
        qInfo() << "[DiskEncryptSetupPrivate::processPendingConfigChange] Executing pending change from actual state" << currentTargetValue
                << "to" << pendingTargetValue;
        hasPendingConfigChange = false;
        currentTargetValue = pendingTargetValue;
        handleOverlayDMModeChangeAsync(pendingTargetValue);
    }
}
