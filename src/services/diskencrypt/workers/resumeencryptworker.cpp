// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resumeencryptworker.h"
#include "core/cryptsetup.h"
#include "helpers/inhibithelper.h"
#include "helpers/jobfilehelper.h"
#include "helpers/abrecoveryhelper.h"
#include "helpers/commonhelper.h"
#include "helpers/crypttabhelper.h"
#include "helpers/notificationhelper.h"

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
    qInfo() << "about to resume encryption...";

    auto fd = inhibit_helper::inhibit(tr("Encrypting..."));

    job_file_helper::loadEncryptJobFile(&m_jobArgs);
    if (m_jobArgs.jobFile.isEmpty()) {
        qInfo() << "no unfinished reencrypt job.";
        return;
    }

    auto status = crypt_setup_helper::encryptStatus(m_jobArgs.devPath);
    if (!(status & disk_encrypt::EncryptState::kStatusEncrypt)) {
        qInfo() << "device is not under encrypting, give up." << m_jobArgs.devPath;
        return;
    }

    m_args.insert(disk_encrypt::encrypt_param_keys::kKeyDevice, m_jobArgs.devPath);
    m_args.insert(disk_encrypt::encrypt_param_keys::kKeyDeviceName, m_jobArgs.devName);

    if (m_jobArgs.devType == disk_encrypt::job_type::TypeFstab) {
        abrecovery_helper::disableRecovery();
    }

    if (!waitForAuthInfo()) {
        setExitCode(-disk_encrypt::kIgnoreRequest);
        return;
    }

    int r = crypt_setup::csResumeEncrypt(m_jobArgs.devPath,
                                         m_jobArgs.volume,
                                         m_jobArgs.devName);
    if (r < 0) {
        setExitCode(r);
        return;
    }

    setPhyDevLabel();
    setPassphrase();
    setRecoveryKey();
    updateCryptTab();
    job_file_helper::removeJobFile(m_jobArgs.jobFile);

    qInfo() << "device encrypt finished." << m_jobArgs.devPath;
}

bool ResumeEncryptWorker::waitForAuthInfo()
{
    while (1) {
        {
            QReadLocker lk(&m_lock);

            if (m_ignoreFlag)
                return false;
            if (!m_authArgs.device.isEmpty()
                && !m_authArgs.passphrase.isEmpty())
                return true;
        }

        Q_EMIT requestAuthInfo({ { disk_encrypt::encrypt_param_keys::kKeyDevice, m_jobArgs.devPath },
                                 { disk_encrypt::encrypt_param_keys::kKeyDeviceName, m_jobArgs.devName } });
        qInfo() << "wait for secret info..." << m_jobArgs.devPath;
        QThread::sleep(3);
    }
    return false;
}

void ResumeEncryptWorker::setPassphrase()
{
    int r = crypt_setup::csChangePassphrase(m_jobArgs.devPath,
                                            "",
                                            m_authArgs.passphrase);
    if (r < 0) {
        qWarning() << "set password failed!" << m_jobArgs.devPath << r;
        return;
    }

    if (!m_authArgs.tpmToken.isEmpty()) {
        auto doc = QJsonDocument::fromJson(m_authArgs.tpmToken.toLocal8Bit());
        auto obj = doc.object();
        obj.insert("keyslots", QJsonArray::fromStringList({ QString::number(r) }));
        doc.setObject(obj);
        auto tk = doc.toJson(QJsonDocument::Compact);
        r = crypt_setup_helper::setToken(m_jobArgs.devPath, tk);
        if (r < 0) {
            qWarning() << "token set failed!" << m_jobArgs.devPath << r;
            return;
        }
    }
    qInfo() << "passphrase setted." << m_jobArgs.devPath;
}

void ResumeEncryptWorker::setRecoveryKey()
{
    if (m_authArgs.recoveryPath.isEmpty())
        return;

    m_authArgs.recoveryKey = common_helper::genRecoveryKey();
    int r = crypt_setup::csAddPassphrase(m_jobArgs.devPath,
                                         m_authArgs.passphrase,
                                         m_authArgs.recoveryKey);
    if (r < 0) {
        qWarning() << "cannot add recovery key!" << m_jobArgs.devPath;
        return;
    }
    QString token = QString("{ 'type': 'usec-recoverykey', 'keyslots': ['%1'] }").arg(r);
    r = crypt_setup_helper::setToken(m_jobArgs.devPath, token);
    if (r < 0) {
        qWarning() << "recovery token set failed!";
        return;
    }

    qInfo() << "recovery key setted." << m_jobArgs.devPath;

    saveRecoveryKey();
}

void ResumeEncryptWorker::setPhyDevLabel()
{
    int r = crypt_setup::csSetLabel(m_jobArgs.devPath, m_jobArgs.devName);
    if (r < 0) {
        qWarning() << "cannot set label!" << m_jobArgs.devPath << m_jobArgs.devName;
        return;
    }
    qInfo() << "device label setted." << m_jobArgs.devPath << m_jobArgs.devName;
}

void ResumeEncryptWorker::updateCryptTab()
{
    if (m_authArgs.tpmToken.isEmpty())
        return;
    crypttab_helper::addCryptOption(m_jobArgs.clearDev, "tpm2-device=auto");
}

void ResumeEncryptWorker::saveRecoveryKey()
{
    setExitCode(-disk_encrypt::KErrorRequestExportRecKey);
    if (!QDir(m_authArgs.recoveryPath).exists()) {
        qWarning() << "export to path does not exist!" << m_authArgs.recoveryPath;
        return;
    }

    auto fileName = QString("%1/%2_recovery_key.txt")
                            .arg(m_authArgs.recoveryPath)
                            .arg(m_authArgs.device.mid(5));
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "cannot create recovery file at" << m_authArgs.recoveryKey;
        return;
    }
    f.write(m_authArgs.recoveryKey.toLocal8Bit());
    f.flush();
    f.close();
    qInfo() << "recovery key saved." << m_jobArgs.devPath;
    setExitCode(disk_encrypt::kSuccess);
}
