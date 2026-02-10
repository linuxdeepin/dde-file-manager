// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resumeencryptworker.h"
#include "core/cryptsetup.h"
#include "core/dmsetup.h"
#include "helpers/inhibithelper.h"
#include "helpers/jobfilehelper.h"
#include "helpers/abrecoveryhelper.h"
#include "helpers/commonhelper.h"
#include "helpers/crypttabhelper.h"
#include "helpers/notificationhelper.h"
#include "helpers/blockdevhelper.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>

FILE_ENCRYPT_USE_NS

ResumeEncryptWorker::ResumeEncryptWorker(const QVariantMap &args, QObject *parent)
    : BaseEncryptWorker(args, parent)
{
    connect(NotificationHelper::instance(), &NotificationHelper::ignoreAuthSetup,
            this, &ResumeEncryptWorker::ignoreAuthRequest);
    connect(NotificationHelper::instance(), &NotificationHelper::replyAuthArgs,
            this, &ResumeEncryptWorker::setAuthInfo);
}

QString ResumeEncryptWorker::recoveryKey()
{
    return m_authArgs.recoveryKey;
}

void ResumeEncryptWorker::setAuthInfo(const QVariantMap &args)
{
    QWriteLocker l(&m_lock);
    using namespace disk_encrypt::encrypt_param_keys;
    m_authArgs.device = args.value(kKeyDevice).toString();
    m_authArgs.passphrase = disk_encrypt::fromBase64(args.value(kKeyPassphrase).toString());
    m_authArgs.tpmToken = args.value(kKeyTPMToken).toString();
    m_authArgs.recoveryPath = args.value(kKeyExportToPath).toString();
}

void ResumeEncryptWorker::ignoreAuthRequest()
{
    QWriteLocker l(&m_lock);
    m_ignoreFlag = true;
}

void ResumeEncryptWorker::run()
{
    qInfo() << "[ResumeEncryptWorker::run] Starting resume encrypt operation";

    job_file_helper::loadEncryptJobFile(&m_jobArgs);
    if (m_jobArgs.jobFile.isEmpty()) {
        qInfo() << "[ResumeEncryptWorker::run] No unfinished reencrypt job found, checking device";
        loadJobFromDevice();
        if (m_jobArgs.devPath.isEmpty()) {
            qInfo() << "[ResumeEncryptWorker::run] No encryption job to resume";
            return;
        }
    }
    qInfo() << "[ResumeEncryptWorker::run] Resuming encryption for device:" << m_jobArgs.devPath;

    auto fd = inhibit_helper::inhibit(tr("Encrypting ") + m_jobArgs.devPath);

    auto status = crypt_setup_helper::encryptStatus(m_jobArgs.devPath);
    if (!(status & disk_encrypt::EncryptState::kStatusEncrypt)) {
        qInfo() << "[ResumeEncryptWorker::run] Device is not under encrypting, giving up, device:" << m_jobArgs.devPath << "status:" << status;
        return;
    }
    qDebug() << "[ResumeEncryptWorker::run] Device encryption status:" << status;

    m_args.insert(disk_encrypt::encrypt_param_keys::kKeyDevice, m_jobArgs.devPath);
    m_args.insert(disk_encrypt::encrypt_param_keys::kKeyDeviceName, m_jobArgs.devName);

    if (!waitForAuthInfo()) {
        qWarning() << "[ResumeEncryptWorker::run] Authentication request ignored by user";
        setExitCode(-disk_encrypt::kIgnoreRequest);
        return;
    }

    qInfo() << "[ResumeEncryptWorker::run] Starting encryption resume";
    int r = crypt_setup::csResumeEncrypt(m_jobArgs.devPath,
                                         m_jobArgs.volume,
                                         m_jobArgs.devName);
    if (r < 0) {
        qCritical() << "[ResumeEncryptWorker::run] Resume encrypt failed, device:" << m_jobArgs.devPath << "error:" << r;
        setExitCode(r);
        return;
    }

    // setPhyDevLabel();
    setPassphrase();
    setRecoveryKey();
    updateCryptTab();
    job_file_helper::removeJobFile(m_jobArgs.jobFile);

    qInfo() << "[ResumeEncryptWorker::run] Device encryption finished successfully, device:" << m_jobArgs.devPath;
}

bool ResumeEncryptWorker::waitForAuthInfo()
{
    qDebug() << "[ResumeEncryptWorker::waitForAuthInfo] Waiting for authentication info";
    while (1) {
        {
            QReadLocker lk(&m_lock);

            if (m_ignoreFlag) {
                qInfo() << "[ResumeEncryptWorker::waitForAuthInfo] Authentication ignored by user";
                return false;
            }
            if (!m_authArgs.device.isEmpty()
                && !m_authArgs.passphrase.isEmpty()) {
                qInfo() << "[ResumeEncryptWorker::waitForAuthInfo] Authentication info received";
                return true;
            }
        }

        Q_EMIT requestAuthInfo({ { disk_encrypt::encrypt_param_keys::kKeyDevice, m_jobArgs.devPath },
                                 { disk_encrypt::encrypt_param_keys::kKeyDeviceName, m_jobArgs.devName } });
        qDebug() << "[ResumeEncryptWorker::waitForAuthInfo] Waiting for authentication info, device:" << m_jobArgs.devPath;
        QThread::sleep(3);
    }
    return false;
}

void ResumeEncryptWorker::setPassphrase()
{
    qDebug() << "[ResumeEncryptWorker::setPassphrase] Setting passphrase";
    int r = crypt_setup::csChangePassphrase(m_jobArgs.devPath,
                                            "",
                                            m_authArgs.passphrase);
    if (r < 0) {
        qCritical() << "[ResumeEncryptWorker::setPassphrase] Set passphrase failed, device:" << m_jobArgs.devPath << "error:" << r;
        return;
    }

    if (!m_authArgs.tpmToken.isEmpty()) {
        qInfo() << "[ResumeEncryptWorker::setPassphrase] Setting TPM token";
        auto doc = QJsonDocument::fromJson(m_authArgs.tpmToken.toLocal8Bit());
        auto obj = doc.object();
        obj.insert("keyslots", QJsonArray::fromStringList({ QString::number(r) }));
        doc.setObject(obj);
        auto tk = doc.toJson(QJsonDocument::Compact);
        r = crypt_setup_helper::setToken(m_jobArgs.devPath, tk);
        if (r < 0) {
            qCritical() << "[ResumeEncryptWorker::setPassphrase] TPM token set failed, device:" << m_jobArgs.devPath << "error:" << r;
            return;
        }
    }
    qInfo() << "[ResumeEncryptWorker::setPassphrase] Passphrase set successfully, device:" << m_jobArgs.devPath;
}

void ResumeEncryptWorker::setRecoveryKey()
{
    qDebug() << "[ResumeEncryptWorker::setRecoveryKey] Setting recovery key";
    if (m_authArgs.recoveryPath.isEmpty()) {
        qDebug() << "[ResumeEncryptWorker::setRecoveryKey] Recovery path is empty, skipping recovery key setup";
        return;
    }

    qInfo() << "[ResumeEncryptWorker::setRecoveryKey] Generating recovery key";
    m_authArgs.recoveryKey = common_helper::genRecoveryKey();
    int r = crypt_setup::csAddPassphrase(m_jobArgs.devPath,
                                         m_authArgs.passphrase,
                                         m_authArgs.recoveryKey);
    if (r < 0) {
        qCritical() << "[ResumeEncryptWorker::setRecoveryKey] Cannot add recovery key, device:" << m_jobArgs.devPath << "error:" << r;
        return;
    }
    QString token = QString("{ 'type': 'usec-recoverykey', 'keyslots': ['%1'] }").arg(r);
    r = crypt_setup_helper::setToken(m_jobArgs.devPath, token);
    if (r < 0) {
        qCritical() << "[ResumeEncryptWorker::setRecoveryKey] Recovery token set failed, device:" << m_jobArgs.devPath << "error:" << r;
        return;
    }

    qInfo() << "[ResumeEncryptWorker::setRecoveryKey] Recovery key set successfully, device:" << m_jobArgs.devPath;

    saveRecoveryKey();
}

void ResumeEncryptWorker::setPhyDevLabel()
{
    qDebug() << "[ResumeEncryptWorker::setPhyDevLabel] Setting physical device label";
    int r = crypt_setup::csSetLabel(m_jobArgs.devPath, m_jobArgs.devName);
    if (r < 0) {
        qCritical() << "[ResumeEncryptWorker::setPhyDevLabel] Cannot set label, device:" << m_jobArgs.devPath << "name:" << m_jobArgs.devName << "error:" << r;
        return;
    }
    qInfo() << "[ResumeEncryptWorker::setPhyDevLabel] Device label set successfully, device:" << m_jobArgs.devPath << "label:" << m_jobArgs.devName;
}

void ResumeEncryptWorker::updateCryptTab()
{
    qDebug() << "[ResumeEncryptWorker::updateCryptTab] Updating crypttab";
    if (m_authArgs.tpmToken.isEmpty()) {
        qDebug() << "[ResumeEncryptWorker::updateCryptTab] TPM token is empty, skipping crypttab update";
        return;
    }
    crypttab_helper::addCryptOption(m_jobArgs.volume, "tpm2-device=auto");
    qInfo() << "[ResumeEncryptWorker::updateCryptTab] Crypttab updated with TPM option";
}

void ResumeEncryptWorker::saveRecoveryKey()
{
    qDebug() << "[ResumeEncryptWorker::saveRecoveryKey] Saving recovery key";
    setExitCode(-disk_encrypt::KErrorRequestExportRecKey);
    if (!QDir(m_authArgs.recoveryPath).exists()) {
        qCritical() << "[ResumeEncryptWorker::saveRecoveryKey] Export path does not exist:" << m_authArgs.recoveryPath;
        return;
    }

    auto fileName = QString("%1/%2_recovery_key.txt")
                            .arg(m_authArgs.recoveryPath)
                            .arg(m_authArgs.device.mid(5));
    qDebug() << "[ResumeEncryptWorker::saveRecoveryKey] Saving recovery key to:" << fileName;

    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "[ResumeEncryptWorker::saveRecoveryKey] Cannot create recovery file:" << fileName;
        return;
    }
    f.write(m_authArgs.recoveryKey.toLocal8Bit());
    f.flush();
    f.close();
    qInfo() << "[ResumeEncryptWorker::saveRecoveryKey] Recovery key saved successfully, device:" << m_jobArgs.devPath;
    setExitCode(disk_encrypt::kSuccess);
}

void ResumeEncryptWorker::loadJobFromDevice()
{
    qDebug() << "[ResumeEncryptWorker::loadJobFromDevice] Loading job from device";
    auto dev = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString();
    if (dev.startsWith("/dev/dm-")) {
        dev = dm_setup_helper::findHolderDev(dev);
        qDebug() << "[ResumeEncryptWorker::loadJobFromDevice] Resolved physical device:" << dev;
    }

    auto ptr = blockdev_helper::createDevPtr(dev);
    if (!ptr) {
        qCritical() << "[ResumeEncryptWorker::loadJobFromDevice] Cannot create device object:" << dev;
        return;
    }
    auto name = ptr->idLabel();

    auto status = crypt_setup_helper::encryptStatus(dev);
    if (status & disk_encrypt::kStatusEncrypt
        && status & disk_encrypt::kStatusOnline) {
        m_jobArgs.devPath = dev;
        m_jobArgs.devName = name;

        qInfo() << "[ResumeEncryptWorker::loadJobFromDevice] Found encryption job from device header, device:" << dev << "name:" << name;
    }
}
