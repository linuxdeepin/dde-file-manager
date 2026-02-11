// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dminitencryptworker.h"
#include "core/dmsetup.h"
#include "core/cryptsetup.h"
#include "helpers/blockdevhelper.h"
#include "helpers/inhibithelper.h"
#include "helpers/crypttabhelper.h"
#include "helpers/cryptsetupcompabilityhelper.h"

#include <sys/mount.h>

#define RetOnFail(ret, msg)    \
    {                          \
        int r = ret;           \
        if (r < 0) {           \
            setExitCode(r);    \
            qCritical() << "[DMInitEncryptWorker::run]" << msg << "error:" << r; \
            return;            \
        }                      \
    }

#define ESuspend QString("Error when suspending dm device")
#define ECreatDM QString("Error when creating dm device")
#define EResume QString("Error when resuming dm device")
#define EReload QString("Error when reloading dm device")
#define EInitEnc QString("Error when initializing encryption")
#define EActive QString("Error when activating device")

FILE_ENCRYPT_USE_NS

DMInitEncryptWorker::DMInitEncryptWorker(const QVariantMap &args, QObject *parent)
    : BaseEncryptWorker(args, parent)
{
}

void DMInitEncryptWorker::run()
{
    if (!CryptSetupCompabilityHelper::instance()->initWithPreProcess()) {
        qCritical() << "[DMInitEncryptWorker::run] Key function not provided by cryptsetup, cannot encrypt with overlay method";
        return;
    }

    qInfo() << "[DMInitEncryptWorker::run] Starting DM overlay encryption initialization";

    const QString &devPath = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString();   // /dev/dm-0
    qInfo() << "[DMInitEncryptWorker::run] About to encrypt overlay device:" << devPath;

    auto fd = inhibit_helper::inhibit(tr("Initialize encryption ") + devPath);

    auto topName = blockdev_helper::getUSecName(devPath);   // usec-overlay-xxxx
    auto midName = QString(topName).replace("overlay", "overlay-mid");
    auto unlockName = QString(topName).replace("overlay", "overlay-unlock");
    qDebug() << "[DMInitEncryptWorker::run] Device names - top:" << topName << ", mid:" << midName << ", unlock:" << unlockName;

    QString phyPath = dm_setup_helper::findHolderDev(devPath);
    qInfo() << "[DMInitEncryptWorker::run] Physical device path:" << phyPath;

    auto phyPtr = blockdev_helper::createDevPtr(phyPath);
    auto source = phyPtr
            ? "PARTUUID=" + phyPtr->getProperty(dfmmount::Property::kPartitionUUID).toString()
            : phyPath;
    qDebug() << "[DMInitEncryptWorker::run] Crypttab source:" << source;
    crypttab_helper::insertCryptItem({ unlockName, source, "none", { "luks", "initramfs", "keyscript=/lib/usec-crypt-kit/usec-askpass" } });
    qInfo() << "[DMInitEncryptWorker::run] Crypttab item inserted for unlock device:" << unlockName;

    // now we can do encrypt on phyDevPath
    auto jobArgs = initJobArgs(phyPath, unlockName);
    job_file_helper::createEncryptJobFile(jobArgs);
    qInfo() << "[DMInitEncryptWorker::run] Encrypt job file created";

    auto _dev = phyPath.toStdString();
    auto _topName = topName.toStdString();
    auto _midName = midName.toStdString();
    const char *argv[] = { _dev.c_str(), _topName.c_str(), _midName.c_str() };
    crypt_setup::CryptPreProcessor proc { .argc = 3, .argv = argv, .proc = detachPhyDevice };

    int r = crypt_setup::csInitEncrypt(phyPath, jobArgs.devName, &proc);
    if (r < 0) {
        qCritical() << "[DMInitEncryptWorker::run]" << EInitEnc << "device:" << phyPath << "error:" << r;
        job_file_helper::removeJobFile(jobArgs.jobFile);
        setExitCode(r);
        return;
    }
    qInfo() << "[DMInitEncryptWorker::run] Overlay encryption initialized successfully, device:" << phyPath;

    r = crypt_setup::csActivateDeviceByVolume(phyPath, unlockName, proc.volumeKey);
    if (r < 0) {
        qWarning() << "[DMInitEncryptWorker::run] Cannot activate device by volume key, trying passphrase, device:" << phyPath << "error:" << r;
        RetOnFail(crypt_setup::csActivateDevice(phyPath, unlockName), EActive + ", device: " + phyPath);
    }
    qInfo() << "[DMInitEncryptWorker::run] Overlay device activated successfully, physical:" << phyPath << ", unlock:" << unlockName;

    // reload midDev to unlocked dev. and resume topDev.
    auto unlockPath = "/dev/mapper/" + unlockName;
    dm_setup::DMTable reloadTab { "linear", unlockPath + " 0", 0, blockdev_helper::devBlockSize(unlockPath) };
    RetOnFail(dm_setup::dmReloadDevice(topName, reloadTab), EReload + ", device: " + topName);
    // RetOnFail(dm_setup::dmResumeDevice(midName), EResume + midName);
    // top dev is suspend in `detachPhyDevice`
    RetOnFail(dm_setup::dmResumeDevice(topName), EResume + ", device: " + topName);

    qInfo() << "[DMInitEncryptWorker::run] Overlay device encryption initialized successfully, device:" << phyPath;
}

job_file_helper::JobDescArgs DMInitEncryptWorker::initJobArgs(const QString &phyDev, const QString &unlockName)
{
    qDebug() << "[DMInitEncryptWorker::initJobArgs] Initializing job arguments";
    job_file_helper::JobDescArgs args;
    auto ptr = blockdev_helper::createDevPtr(phyDev);
    if (!ptr) {
        qCritical() << "[DMInitEncryptWorker::initJobArgs] Cannot create block device for init job desc, device:" << phyDev;
        return args;
    }

    args.device = "PARTUUID=" + ptr->getProperty(dfmmount::Property::kPartitionUUID).toString();
    args.volume = unlockName;
    args.devName = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDeviceName).toString();
    args.devPath = phyDev;
    args.devType = disk_encrypt::job_type::TypeOverlay;
    qDebug() << "[DMInitEncryptWorker::initJobArgs] Job args initialized, device:" << args.device << ", volume:" << args.volume;

    return args;
}

int DMInitEncryptWorker::detachPhyDevice(int argc, const char *argv[])
{
    qDebug() << "[DMInitEncryptWorker::detachPhyDevice] Detaching physical device";
    if (argc < 3) {
        qCritical() << "[DMInitEncryptWorker::detachPhyDevice] Parameter error, argc:" << argc << ", argv[0]:" << *argv;
        return -disk_encrypt::kErrorUnknown;
    }

    QString dev(argv[0]), topName(argv[1]), midName(argv[2]);
    qDebug() << "[DMInitEncryptWorker::detachPhyDevice] Detaching physical device:" << dev << ", top:" << topName << ", mid:" << midName;

    auto devSize = blockdev_helper::devBlockSize(dev);
    qDebug() << "[DMInitEncryptWorker::detachPhyDevice] Device size:" << devSize;
    dm_setup::DMTable initTab { "linear", dev + " 0", 0, devSize };
    dm_setup::DMTable zeroTab { "zero", "", 0, devSize };
    dm_setup::DMTable reloadTab { "linear", "/dev/mapper/" + midName + " 0", 0, devSize };

    qInfo() << "[DMInitEncryptWorker::detachPhyDevice] Starting suspend device:" << topName;

    int r = disk_encrypt::kSuccess;
    do {
        // create midDev
        r = dm_setup::dmSuspendDevice(topName);
        if (r != 0) {
            qWarning() << "[DMInitEncryptWorker::detachPhyDevice] Suspend device failed, device:" << topName << "error:" << r;
            break;
        }

        // physical device already detached.
        qInfo() << "[DMInitEncryptWorker::detachPhyDevice] Physical device detached successfully";
        return disk_encrypt::kSuccess;
    } while (0);

    // dm_setup::dmRemoveDevice(midName);
    dm_setup::dmReloadDevice(topName, initTab);
    dm_setup::dmResumeDevice(topName);
    qWarning() << "[DMInitEncryptWorker::detachPhyDevice] dmsetup failed, reloaded to initial status, device:" << topName << "error:" << r;
    return r;
}
