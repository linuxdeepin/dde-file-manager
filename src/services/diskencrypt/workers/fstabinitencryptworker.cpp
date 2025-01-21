// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globaltypesdefine.h"
#include "fstabinitencryptworker.h"
#include "helpers/blockdevhelper.h"
#include "helpers/inhibithelper.h"
#include "helpers/commonhelper.h"
#include "helpers/fstabhelper.h"

FILE_ENCRYPT_USE_NS

FstabInitEncryptWorker::FstabInitEncryptWorker(const QVariantMap &args, QObject *parent)
    : BaseEncryptWorker(args, parent)
{
}

void FstabInitEncryptWorker::run()
{
    auto fd = inhibit_helper::inhibit(tr("Initialize encryption..."));
    auto devPath = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString();

    auto ptr = blockdev_helper::createDevPtr(devPath);
    if (!ptr) {
        qWarning() << "cannot create device object!" << devPath;
        setExitCode(-disk_encrypt::kErrorUnknown);
        return;
    }

    auto mpt = ptr->mountPoint();
    auto devUUID = ptr->getProperty(dfmmount::Property::kBlockIDUUID).toString();
    if (disk_encrypt::kDisabledEncryptPath.contains(mpt)) {
        qInfo() << "cannot encrypt device which mounted at" << mpt;
        setExitCode(-disk_encrypt::kErrorDisabledMountPoint);
        return;
    }

    fstab_helper::setFstabTimeout(devPath, devUUID);
    common_helper::createRebootFlagFile(devPath);
    job_file_helper::createEncryptJobFile(initJobArgs(devPath));
    setExitCode(-disk_encrypt::kRebootRequired);
}

job_file_helper::JobDescArgs FstabInitEncryptWorker::initJobArgs(const QString &dev)
{
    job_file_helper::JobDescArgs args;

    auto ptr = blockdev_helper::createDevPtr(dev);
    if (!ptr) {
        qWarning() << "cannot create block for init job desc!" << dev;
        return args;
    }

    args.device = "PARTUUID=" + ptr->getProperty(dfmmount::Property::kPartitionUUID).toString();
    args.volume = "dm-" + dev.mid(5);
    args.cipher = common_helper::encryptCipher();
    args.keySze = "256";

    args.devPath = dev;
    args.prefferPath = ptr->getProperty(dfmmount::Property::kBlockPreferredDevice).toString();
    args.devType = disk_encrypt::job_type::TypeFstab;
    args.devName = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDeviceName).toString();

    return args;
}
