// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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

#include <dfm-mount/dmount.h>

#include <QDBusMessage>
#include <QtConcurrent>
#include <QDataStream>

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
}

void DiskEncryptSetupPrivate::initialize()
{
    qInfo() << "[DiskEncryptSetupPrivate::initialize] Starting initialization process";
    QtConcurrent::run([] {
        filesystem_helper::remountBoot();
        common_helper::createDFMDesktopEntry();
        crypttab_helper::mergeCryptTab();
    });
    job_file_helper::checkJobs();
    resumeEncryption();
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
    qptr->lockTimer(true);
    qInfo() << "auto quit timer is locked.";
}

void DiskEncryptSetupPrivate::onLongTimeJobStopped()
{
    jobRunning = false;
    qptr->lockTimer(false);
    qInfo() << "auto quite timer is unlocked";
}
