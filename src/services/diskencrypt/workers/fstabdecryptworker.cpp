// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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
    qInfo() << "about to decrypt fstab device...";

    using namespace disk_encrypt::encrypt_param_keys;
    auto devPath = m_args.value(kKeyDevice, "").toString();

    auto fd = inhibit_helper::inhibit(tr("Decrypting ") + devPath);

    auto status = crypt_setup_helper::encryptStatus(devPath);
    if (status & disk_encrypt::kStatusOnline && status & disk_encrypt::kStatusEncrypt) {
        qWarning() << "device is not fully encrypted, cannot decrypt!" << devPath;
        setExitCode(-disk_encrypt::kErrorNotFullyEncrypted);
        return;
    }

    auto partUUID = partitionUUID(devPath);
    if (partUUID.isEmpty()) {
        qWarning() << "cannot obtain partition of device";
        setExitCode(-disk_encrypt::kErrorUnknown);
        return;
    }
    job_file_helper::createDecryptJobFile({ .device = "PARTUUID=" + partUUID,
                                            .devPath = devPath,
                                            .devType = disk_encrypt::job_type::TypeFstab });
    auto clearUUID = clearDeviceUUID(devPath);
    if (!clearUUID.isEmpty())
        fstab_helper::setFstabPassno("UUID=" + clearUUID, 0);
    common_helper::createRebootFlagFile(devPath);
    setExitCode(-disk_encrypt::kRebootRequired);

    qInfo() << "fstab device decrypt job created, request for reboot.";
}

QString FstabDecryptWorker::clearDeviceUUID(const QString &dev)
{
    setExitCode(-disk_encrypt::kErrorUnknown);
    auto ptr = blockdev_helper::createDevPtr(dev);
    if (!ptr) {
        qWarning() << "cannot create device object!" << dev;
        return "";
    }

    auto clearDev = ptr->getProperty(dfmmount::Property::kEncryptedCleartextDevice).toString();
    auto clearPtr = blockdev_helper::createDevPtr2(clearDev);
    if (!clearPtr) {
        qWarning() << "cannot create device object!" << dev << clearDev;
        return "";
    }

    setExitCode(disk_encrypt::kSuccess);
    return clearPtr->getProperty(dfmmount::Property::kBlockIDUUID).toString();
}

QString FstabDecryptWorker::partitionUUID(const QString &dev)
{
    setExitCode(-disk_encrypt::kErrorUnknown);
    auto ptr = blockdev_helper::createDevPtr(dev);
    if (!ptr) {
        qWarning() << "cannot create device object!" << dev;
        return "";
    }

    setExitCode(disk_encrypt::kSuccess);
    return ptr->getProperty(dfmmount::Property::kPartitionUUID).toString();
}
