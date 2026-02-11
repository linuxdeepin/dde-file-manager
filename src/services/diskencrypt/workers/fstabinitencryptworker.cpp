// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globaltypesdefine.h"
#include "fstabinitencryptworker.h"
#include "helpers/blockdevhelper.h"
#include "helpers/crypttabhelper.h"
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
    qInfo() << "[FstabInitEncryptWorker::run] Starting fstab encryption initialization";

    auto devPath = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString();
    qInfo() << "[FstabInitEncryptWorker::run] About to encrypt fstab device:" << devPath;

    auto fd = inhibit_helper::inhibit(tr("Initialize encryption ") + devPath);

    auto ptr = blockdev_helper::createDevPtr(devPath);
    if (!ptr) {
        qCritical() << "[FstabInitEncryptWorker::run] Cannot create device object:" << devPath;
        setExitCode(-disk_encrypt::kErrorUnknown);
        return;
    }

    auto mpt = ptr->mountPoint();
    auto devUUID = ptr->getProperty(dfmmount::Property::kBlockIDUUID).toString();
    qDebug() << "[FstabInitEncryptWorker::run] Device mount point:" << mpt << ", UUID:" << devUUID;

    if (disk_encrypt::kDisabledEncryptPath.contains(mpt)) {
        qWarning() << "[FstabInitEncryptWorker::run] Cannot encrypt device mounted at disabled path, mount point:" << mpt;
        setExitCode(-disk_encrypt::kErrorDisabledMountPoint);
        return;
    }

    fstab_helper::setFstabTimeout(devPath, devUUID);
    common_helper::createRebootFlagFile(devPath);

    // 设备存在 dm 堆叠的时候，也可能会启用 fstab 方式的加密动作，所以这里要保存物理设备路径，以便加密可以正常进行
    // 设备在经过初始化之后，会叠加一层 dm 设备，以便可以在系统运行时将其取消加密。此时再次触发加密，会按需走到 fstab 加密流程
    auto phyPath = m_args.value(disk_encrypt::encrypt_param_keys::kKeyPhyDevice).toString();
    if (phyPath.isEmpty()) phyPath = devPath;
    qDebug() << "[FstabInitEncryptWorker::run] Physical device path:" << phyPath;

    auto jobArgs = initJobArgs(phyPath);
    job_file_helper::createEncryptJobFile(jobArgs);
    qInfo() << "[FstabInitEncryptWorker::run] Encrypt job file created";

    auto phyPtr = blockdev_helper::createDevPtr(phyPath);
    auto source = phyPtr
            ? "PARTUUID=" + phyPtr->getProperty(dfmmount::Property::kPartitionUUID).toString()
            : devPath;
    qDebug() << "[FstabInitEncryptWorker::run] Crypttab source:" << source;
    crypttab_helper::insertCryptItem({ jobArgs.volume, source, "none", { "luks", "initramfs", "keyscript=/lib/usec-crypt-kit/usec-askpass" } });
    qInfo() << "[FstabInitEncryptWorker::run] Crypttab item inserted";

    setExitCode(-disk_encrypt::kRebootRequired);

    qInfo() << "[FstabInitEncryptWorker::run] Fstab device encrypt job created, reboot required";
}

job_file_helper::JobDescArgs FstabInitEncryptWorker::initJobArgs(const QString &dev)
{
    qDebug() << "[FstabInitEncryptWorker::initJobArgs] Initializing job arguments";
    job_file_helper::JobDescArgs args;

    auto ptr = blockdev_helper::createDevPtr(dev);
    if (!ptr) {
        qCritical() << "[FstabInitEncryptWorker::initJobArgs] Cannot create block device for init job desc, device:" << dev;
        return args;
    }

    args.device = "PARTUUID=" + ptr->getProperty(dfmmount::Property::kPartitionUUID).toString();
    args.volume = "usec-overlay-unlock-" + dev.mid(5);
    args.cipher = common_helper::encryptCipher() + "-xts-plain64";
    args.keySze = "256";

    args.devPath = dev;
    args.prefferPath = ptr->getProperty(dfmmount::Property::kBlockPreferredDevice).toString();
    args.devType = disk_encrypt::job_type::TypeFstab;
    args.devName = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDeviceName).toString();

    qDebug() << "[FstabInitEncryptWorker::initJobArgs] Job args initialized, device:" << args.device << ", volume:" << args.volume;

    return args;
}
