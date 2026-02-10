// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fstabdecryptworker.h"
#include "core/cryptsetup.h"
#include "helpers/fstabhelper.h"
#include "helpers/inhibithelper.h"
#include "helpers/jobfilehelper.h"
#include "helpers/blockdevhelper.h"
#include "helpers/commonhelper.h"

FILE_ENCRYPT_USE_NS

FstabDecryptWorker::FstabDecryptWorker(const QVariantMap &args, QObject *parent)
    : BaseEncryptWorker(args, parent)
{
}

void FstabDecryptWorker::run()
{
    qInfo() << "[FstabDecryptWorker::run] Starting fstab decryption";

    using namespace disk_encrypt::encrypt_param_keys;
    auto devPath = m_args.value(kKeyDevice, "").toString();
    qInfo() << "[FstabDecryptWorker::run] About to decrypt fstab device:" << devPath;

    auto fd = inhibit_helper::inhibit(tr("Decrypting ") + devPath);

    auto status = crypt_setup_helper::encryptStatus(devPath);
    if (status & disk_encrypt::kStatusOnline && status & disk_encrypt::kStatusEncrypt) {
        qCritical() << "[FstabDecryptWorker::run] Device is not fully encrypted, cannot decrypt, device:" << devPath << "status:" << status;
        setExitCode(-disk_encrypt::kErrorNotFullyEncrypted);
        return;
    }
    qDebug() << "[FstabDecryptWorker::run] Device encryption status:" << status;

    auto partUUID = partitionUUID(devPath);
    if (partUUID.isEmpty()) {
        qCritical() << "[FstabDecryptWorker::run] Cannot obtain partition UUID of device:" << devPath;
        setExitCode(-disk_encrypt::kErrorUnknown);
        return;
    }
    qDebug() << "[FstabDecryptWorker::run] Partition UUID:" << partUUID;

    job_file_helper::JobDescArgs jobArgs {.device = "PARTUUID=" + partUUID,
                                           .devPath = devPath,
                                           .devType = disk_encrypt::job_type::TypeFstab };
    job_file_helper::createDecryptJobFile(jobArgs);
    qInfo() << "[FstabDecryptWorker::run] Decrypt job file created";

    auto clearUUID = clearDeviceUUID(devPath);
    if (!clearUUID.isEmpty()) {
        fstab_helper::setFstabPassno("UUID=" + clearUUID, 0);
        qDebug() << "[FstabDecryptWorker::run] Fstab passno set for clear device UUID:" << clearUUID;
    }

    common_helper::createRebootFlagFile(devPath);
    setExitCode(-disk_encrypt::kRebootRequired);

    qInfo() << "[FstabDecryptWorker::run] Fstab device decrypt job created, reboot required";
}

QString FstabDecryptWorker::clearDeviceUUID(const QString &dev)
{
    qDebug() << "[FstabDecryptWorker::clearDeviceUUID] Getting clear device UUID";
    setExitCode(-disk_encrypt::kErrorUnknown);
    auto ptr = blockdev_helper::createDevPtr(dev);
    if (!ptr) {
        qCritical() << "[FstabDecryptWorker::clearDeviceUUID] Cannot create device object:" << dev;
        return "";
    }

    auto clearDev = ptr->getProperty(dfmmount::Property::kEncryptedCleartextDevice).toString();
    qDebug() << "[FstabDecryptWorker::clearDeviceUUID] Clear device path:" << clearDev;

    auto clearPtr = blockdev_helper::createDevPtr2(clearDev);
    if (!clearPtr) {
        qCritical() << "[FstabDecryptWorker::clearDeviceUUID] Cannot create clear device object, device:" << dev << "clear device:" << clearDev;
        return "";
    }

    setExitCode(disk_encrypt::kSuccess);
    auto uuid = clearPtr->getProperty(dfmmount::Property::kBlockIDUUID).toString();
    qDebug() << "[FstabDecryptWorker::clearDeviceUUID] Clear device UUID:" << uuid;
    return uuid;
}

QString FstabDecryptWorker::partitionUUID(const QString &dev)
{
    qDebug() << "[FstabDecryptWorker::partitionUUID] Getting partition UUID";
    setExitCode(-disk_encrypt::kErrorUnknown);
    auto ptr = blockdev_helper::createDevPtr(dev);
    if (!ptr) {
        qCritical() << "[FstabDecryptWorker::clearDeviceUUID] Cannot create device object:" << dev;
        return "";
    }

    setExitCode(disk_encrypt::kSuccess);
    auto uuid = ptr->getProperty(dfmmount::Property::kPartitionUUID).toString();
    qDebug() << "[FstabDecryptWorker::partitionUUID] Partition UUID:" << uuid;
    return uuid;
}
