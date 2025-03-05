// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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
    qInfo() << "about to encrypt normal device...";

    auto devPath = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString();
    auto fd = inhibit_helper::inhibit(tr("Initialize encryption ") + devPath);

    auto blkPtr = blockdev_helper::createDevPtr(devPath);
    if (blkPtr && !blkPtr->mountPoints().isEmpty()) {
        setExitCode(-disk_encrypt::kErrorDeviceMounted);
        return;
    }

    int r = crypt_setup::csInitEncrypt(devPath,
                                       m_args.value(disk_encrypt::encrypt_param_keys::kKeyDeviceName).toString());
    if (r < 0) {
        setExitCode(r);
        return;
    }

    auto jobArgs = initJobArgs(blkPtr);
    job_file_helper::createEncryptJobFile(jobArgs);
    setExitCode(disk_encrypt::kSuccess);
    qInfo() << "normal device encryption inited." << devPath;
}

job_file_helper::JobDescArgs NormalInitEncryptWorker::initJobArgs(DevPtr ptr)
{
    job_file_helper::JobDescArgs args;
    args.device = "PARTUUID=" + ptr->getProperty(dfmmount::Property::kPartitionUUID).toString();
    args.devType = disk_encrypt::job_type::TypeNormal;
    args.devPath = ptr->device();
    args.devName = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDeviceName).toString();
    return args;
}
