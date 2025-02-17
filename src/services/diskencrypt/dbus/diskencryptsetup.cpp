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

#include <dfm-mount/dmount.h>

#include <QDBusMessage>
#include <QtConcurrent>

#include <polkit-qt5-1/PolkitQt1/Authority>

static constexpr char kActionEncrypt[] { "org.deepin.Filemanager.DiskEncrypt.Encrypt" };
static constexpr char kActionDecrypt[] { "org.deepin.Filemanager.DiskEncrypt.Decrypt" };
static constexpr char kActionChgPwd[] { "org.deepin.Filemanager.DiskEncrypt.ChangePassphrase" };

FILE_ENCRYPT_USE_NS

DiskEncryptSetup::DiskEncryptSetup(QObject *parent)
    : QDBusService(parent),
      QDBusContext(),
      m_dptr(new DiskEncryptSetupPrivate(this))
{
    initPolicy(QDBusConnection::SystemBus,
               QString(SERVICE_CONFIG_DIR) + "other/deepin-diskencrypt-service.json");
    dfmmount::DDeviceManager::instance();
    m_dptr->initialize();

    connect(NotificationHelper::instance(), &NotificationHelper::notifyEncryptProgress,
            this, &DiskEncryptSetup::EncryptProgress);
    connect(NotificationHelper::instance(), &NotificationHelper::notifyDecryptProgress,
            this, &DiskEncryptSetup::DecryptProgress);
}

void DiskEncryptSetup::InitEncryption(const QVariantMap &args)
{
    if (m_dptr->jobRunning) {
        qInfo() << "has processing job. cannot create a new job yet.";
        return;
    }

    if (!m_dptr->checkAuth(kActionEncrypt))
        return;

    if (!m_dptr->validateInitArgs(args)) {
        qWarning() << "the initialize args is not valid!";
        return;
    }

    auto type = args.value(disk_encrypt::encrypt_param_keys::kKeyJobType).toString();
    auto worker = m_dptr->createInitWorker(type, args);
    if (!worker) return;
    m_dptr->initThreadConnection(worker);
    connect(worker, &QThread::finished,
            m_dptr, &DiskEncryptSetupPrivate::onInitEncryptFinished);
    worker->start();
}

void DiskEncryptSetup::ResumeEncryption(const QVariantMap &args)
{
    if (m_dptr->jobRunning) {
        qInfo() << "has processing job. cannot create a new job yet.";
        return;
    }

    if (!m_dptr->validateResumeArgs(args)) {
        qWarning() << "the resume args is not valid!";
        return;
    }
    m_dptr->resumeEncryption();
}

void DiskEncryptSetup::Decryption(const QVariantMap &args)
{
    if (m_dptr->jobRunning) {
        qInfo() << "has processing job. cannot create a new job yet.";
        return;
    }

    if (!m_dptr->checkAuth(kActionDecrypt))
        return;

    if (!m_dptr->validateDecryptArgs(args)) {
        qWarning() << "the decrytion args is not valid!";
        return;
    }

    auto type = args.value(disk_encrypt::encrypt_param_keys::kKeyJobType).toString();
    auto worker = m_dptr->createDecryptWorker(type, args);
    if (!worker) return;
    m_dptr->initThreadConnection(worker);
    connect(worker, &QThread::finished,
            m_dptr, &DiskEncryptSetupPrivate::onDecryptFinished);
    worker->start();
}

void DiskEncryptSetup::ChangePassphrase(const QVariantMap &args)
{
    if (!m_dptr->checkAuth(kActionChgPwd))
        return;

    if (!m_dptr->validateChgPwdArgs(args)) {
        qWarning() << "the change password args is not valid!";
        return;
    }

    auto worker = new PassphraseChangeWorker(args);
    connect(worker, &QThread::finished,
            m_dptr, &DiskEncryptSetupPrivate::onPassphraseChanged);
    worker->start();
}

void DiskEncryptSetup::SetupAuthArgs(const QVariantMap &args)
{
    Q_EMIT NotificationHelper::instance()->replyAuthArgs(args);
}

void DiskEncryptSetup::IgnoreAuthSetup()
{
    Q_EMIT NotificationHelper::instance()->ignoreAuthSetup();
}

QString DiskEncryptSetup::TpmToken(const QString &dev)
{
    QString token;
    crypt_setup_helper::getToken(dev, &token);
    if (token.isEmpty()) {
        QString phyDev = dm_setup_helper::findHolderDev(dev);
        crypt_setup_helper::getToken(phyDev, &token);
    }
    return token;
}

int DiskEncryptSetup::DeviceStatus(const QString &dev)
{
    // check status of device itself.
    auto status = crypt_setup_helper::encryptStatus(dev);
    if (status != disk_encrypt::kStatusNotEncrypted)
        return status;

    auto phyDev = dm_setup_helper::findHolderDev(dev);
    return crypt_setup_helper::encryptStatus(phyDev);

    // // check the if device in holder chain is encrypted.
    // auto partitions = dm_setup_helper::procPartitions();
    // QList<dm_setup_helper::ProcPartition> holderChain;
    // for (auto part : partitions) {
    // }

    // return disk_encrypt::kStatusNotEncrypted;
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
    job_file_helper::JobDescArgs args;
    job_file_helper::loadDecryptJobFile(&args);
    return args.devPath;
}

DiskEncryptSetupPrivate::DiskEncryptSetupPrivate(DiskEncryptSetup *parent)
    : QObject(parent),
      qptr(parent) { }

void DiskEncryptSetupPrivate::initialize()
{
    QtConcurrent::run([] {
        common_helper::createDFMDesktopEntry();
        crypttab_helper::updateCryptTab();
    });
    resumeEncryption();
}

void DiskEncryptSetupPrivate::resumeEncryption()
{
    auto worker = new ResumeEncryptWorker({});
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

void DiskEncryptSetupPrivate::initThreadConnection(const QThread *thread)
{
    connect(thread, &QThread::started, this, [this] { jobRunning = true; });
    connect(thread, &QThread::finished, this, [this] { jobRunning = false; });
}

void DiskEncryptSetupPrivate::onInitEncryptFinished()
{
    auto worker = dynamic_cast<BaseEncryptWorker *>(sender());
    if (!worker) return;
    worker->deleteLater();

    auto args = worker->args();
    auto code = worker->exitCode();
    if (code == disk_encrypt::kSuccess)
        resumeEncryption();

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
