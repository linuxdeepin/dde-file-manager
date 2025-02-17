// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dminitencryptworker.h"
#include "core/dmsetup.h"
#include "core/cryptsetup.h"
#include "helpers/blockdevhelper.h"
#include "helpers/inhibithelper.h"
#include "helpers/crypttabhelper.h"

#include <sys/mount.h>

#define RetOnFail(ret, msg) \
    if (ret < 0) {          \
        setExitCode(ret);   \
        qWarning() << msg;  \
        return;             \
    }
#define ESuspend "error when SUSPEND dm device"
#define ECreatDM "error when CREATE dm device"
#define EResume "error when RESUME dm device"
#define EReload "error when RELOAD dm device"
#define EInitEnc "error when INITIAL encrytion"
#define EActive "error when ACTIVATE device"

FILE_ENCRYPT_USE_NS

DMInitEncryptWorker::DMInitEncryptWorker(const QVariantMap &args, QObject *parent)
    : BaseEncryptWorker(args, parent)
{
}

void DMInitEncryptWorker::run()
{
    qInfo() << "about to encrypt overlay device...";

    auto fd = inhibit_helper::inhibit(tr("Initialize encryption..."));

    const QString &devPath = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString();   // /dev/dm-0
    auto dev = blockdev_helper::createDevPtr(devPath);
    if (!dev) {
        qWarning() << "cannot create device!" << devPath;
        return;
    }

    QString phyDevPath = dm_setup_helper::findHolderDev(devPath);
    auto topDmName = blockdev_helper::getUSecName(devPath);   // usec-overlay-xxxx

    auto midDmName(topDmName);
    midDmName.replace("overlay", "overlay-mid");   // usec-overlay-mid-xxxx
    auto midDmPath = "/dev/mapper/" + midDmName;

    auto activeDmName(topDmName);
    activeDmName.replace("overlay", "overlay-unlock");   // usec-overlay-unlock-xxxx

    auto phyPtr = blockdev_helper::createDevPtr(phyDevPath);
    auto source = phyPtr
            ? "PARTUUID=" + phyPtr->getProperty(dfmmount::Property::kPartitionUUID).toString()
            : phyDevPath;
    crypttab_helper::insertCryptItem({ activeDmName,
                                       source,
                                       "none",
                                       { "luks", "initramfs" } });

    // create a new dm device
    RetOnFail(dm_setup::dmSuspendDevice(topDmName), ESuspend);
    auto r = dm_setup::dmCreateDevice(midDmName,
                                      { "linear", phyDevPath + " 0", 0, blockdev_helper::devBlockSize(phyDevPath) });
    if (r < 0) {
        qWarning() << ECreatDM << midDmPath << phyDevPath;
        setExitCode(r);
        r = dm_setup::dmResumeDevice(topDmName);
        qWarning() << "try resume suspended device:" << topDmName << r;
        return;
    }
    RetOnFail(dm_setup::dmResumeDevice(topDmName), EResume);

    // avoid new created dm device automatically mounted.
    umount(midDmPath.toStdString().c_str());

    // reload top dm dev to mid dm dev.
    RetOnFail(dm_setup::dmSuspendDevice(topDmName), ESuspend);
    r = dm_setup::dmReloadDevice(topDmName,
                                 { "linear", midDmPath + " 0", 0, blockdev_helper::devBlockSize(midDmPath) });
    if (r < 0) {
        qWarning() << EReload << midDmPath << phyDevPath;
        setExitCode(r);
        r = dm_setup::dmResumeDevice(topDmName);
        qWarning() << "try resume suspended device:" << topDmName << r;
        return;
    }
    RetOnFail(dm_setup::dmResumeDevice(topDmName), EResume);

    // reload mid dm to zero dm dev
    RetOnFail(dm_setup::dmSuspendDevice(midDmName), ESuspend);
    r = dm_setup::dmReloadDevice(midDmName,
                                 { "zero", "", 0, blockdev_helper::devBlockSize(midDmPath) });
    if (r < 0) {
        qWarning() << EReload << midDmPath << phyDevPath;
        setExitCode(r);
        r = dm_setup::dmResumeDevice(midDmName);
        qWarning() << "try resume suspended device:" << topDmName << r;
        return;
    }
    RetOnFail(dm_setup::dmResumeDevice(midDmName), EResume);

    // now we can do encrypt on phyDevPath
    RetOnFail(crypt_setup::csInitEncrypt(phyDevPath), EInitEnc);
    job_file_helper::createEncryptJobFile(initJobArgs(phyDevPath));

    // open crypt device
    RetOnFail(crypt_setup::csActivateDevice(phyDevPath, activeDmName), EActive);

    // reload mid dm to activeDmPath
    auto activeDmPath = "/dev/mapper/" + activeDmName;
    RetOnFail(dm_setup::dmSuspendDevice(midDmName), ESuspend);
    r = dm_setup::dmReloadDevice(midDmName,
                                 { "linear", activeDmPath + " 0", 0, blockdev_helper::devBlockSize(activeDmPath) });
    if (r < 0) {
        qWarning() << EReload << midDmPath << phyDevPath;
        setExitCode(r);
        r = dm_setup::dmResumeDevice(midDmName);
        qWarning() << "try resume suspended device:" << topDmName << r;
        return;
    }
    RetOnFail(dm_setup::dmResumeDevice(midDmName), EResume);

    crypttab_helper::updateInitramfs();

    qInfo() << "overlay device encryption inited." << phyDevPath;
}

job_file_helper::JobDescArgs DMInitEncryptWorker::initJobArgs(const QString &phyDev)
{
    job_file_helper::JobDescArgs args;
    auto ptr = blockdev_helper::createDevPtr(phyDev);
    if (!ptr) {
        qWarning() << "cannot create block for init job desc!" << phyDev;
        return args;
    }

    args.device = "PARTUUID=" + ptr->getProperty(dfmmount::Property::kPartitionUUID).toString();
    args.volume = "usec-overlay-unlock-" + phyDev.mid(5);
    args.devName = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDeviceName).toString();
    args.devPath = phyDev;
    args.devType = disk_encrypt::job_type::TypeOverlay;

    return args;
}
