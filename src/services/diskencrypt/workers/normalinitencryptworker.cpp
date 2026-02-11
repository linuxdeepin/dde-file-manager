// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "normalinitencryptworker.h"
#include "core/cryptsetup.h"
#include "helpers/inhibithelper.h"
#include "helpers/blockdevhelper.h"
#include "helpers/jobfilehelper.h"

FILE_ENCRYPT_USE_NS

NormalInitEncryptWorker::NormalInitEncryptWorker(const QVariantMap &args, QObject *parent)
    : BaseEncryptWorker(args, parent)
{
}

void NormalInitEncryptWorker::run()
{
    qInfo() << "[NormalInitEncryptWorker::run] Starting normal device encryption initialization";

    auto devPath = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString();
    qInfo() << "[NormalInitEncryptWorker::run] About to encrypt normal device:" << devPath;

    auto fd = inhibit_helper::inhibit(tr("Initialize encryption ") + devPath);

    auto blkPtr = blockdev_helper::createDevPtr(devPath);
    if (blkPtr && !blkPtr->mountPoints().isEmpty()) {
        qInfo() << "[NormalInitEncryptWorker::run] Device is mounted, attempting to unmount:" << devPath;
        if (!blkPtr->unmount()) {
            qCritical() << "[NormalInitEncryptWorker::run] Cannot unmount device, encryption stopped, device:" << devPath;
            setExitCode(-disk_encrypt::kErrorDeviceMounted);
            return;
        }
        qInfo() << "[NormalInitEncryptWorker::run] Device unmounted successfully:" << devPath;
    }

    int r = crypt_setup::csInitEncrypt(devPath,
                                       m_args.value(disk_encrypt::encrypt_param_keys::kKeyDeviceName).toString());
    if (r < 0) {
        qCritical() << "[NormalInitEncryptWorker::run] Init encrypt failed, device:" << devPath << "error:" << r;
        setExitCode(r);
        return;
    }

    auto jobArgs = initJobArgs(blkPtr);
    job_file_helper::createEncryptJobFile(jobArgs);
    setExitCode(disk_encrypt::kSuccess);
    qInfo() << "[NormalInitEncryptWorker::run] Normal device encryption initialized successfully, device:" << devPath;

    sleep(1);
    system("udevadm trigger");
}

job_file_helper::JobDescArgs NormalInitEncryptWorker::initJobArgs(DevPtr ptr)
{
    qDebug() << "[NormalInitEncryptWorker::initJobArgs] Initializing job arguments";
    job_file_helper::JobDescArgs args;
    args.device = "PARTUUID=" + ptr->getProperty(dfmmount::Property::kPartitionUUID).toString();
    args.devType = disk_encrypt::job_type::TypeNormal;
    args.devPath = ptr->device();
    args.devName = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDeviceName).toString();
    qDebug() << "[NormalInitEncryptWorker::initJobArgs] Job args initialized, device:" << args.device << ", path:" << args.devPath;
    return args;
}
