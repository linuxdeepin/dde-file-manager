// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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
            qWarning() << msg; \
            return;            \
        }                      \
    }

#define ESuspend QString("error when SUSPEND dm device ")
#define ECreatDM QString("error when CREATE dm devic e")
#define EResume QString("error when RESUME dm device ")
#define EReload QString("error when RELOAD dm device ")
#define EInitEnc QString("error when INITIAL encrytion ")
#define EActive QString("error when ACTIVATE device ")

FILE_ENCRYPT_USE_NS

DMInitEncryptWorker::DMInitEncryptWorker(const QVariantMap &args, QObject *parent)
    : BaseEncryptWorker(args, parent)
{
}

void DMInitEncryptWorker::run()
{
    if (!CryptSetupCompabilityHelper::instance()->initWithPreProcess()) {
        qWarning() << "key function does not provided by cryptsetup!"
                   << "cannot encrypt with overlay method!";
        return;
    }

    qInfo() << "about to encrypt overlay device...";
    const QString &devPath = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString();   // /dev/dm-0
    auto fd = inhibit_helper::inhibit(tr("Initialize encryption ") + devPath);

    auto topName = blockdev_helper::getUSecName(devPath);   // usec-overlay-xxxx
    auto midName = QString(topName).replace("overlay", "overlay-mid");
    auto unlockName = QString(topName).replace("overlay", "overlay-unlock");

    QString phyPath = dm_setup_helper::findHolderDev(devPath);
    auto phyPtr = blockdev_helper::createDevPtr(phyPath);
    auto source = phyPtr
            ? "PARTUUID=" + phyPtr->getProperty(dfmmount::Property::kPartitionUUID).toString()
            : phyPath;
    crypttab_helper::insertCryptItem({ unlockName, source, "none", { "luks", "initramfs", "keyscript=/lib/usec-crypt-kit/usec-askpass" } });

    // now we can do encrypt on phyDevPath
    auto jobArgs = initJobArgs(phyPath, unlockName);
    job_file_helper::createEncryptJobFile(jobArgs);

    auto _dev = phyPath.toStdString();
    auto _topName = topName.toStdString();
    auto _midName = midName.toStdString();
    const char *argv[] = { _dev.c_str(), _topName.c_str(), _midName.c_str() };
    crypt_setup::CryptPreProcessor proc { .argc = 3, .argv = argv, .proc = detachPhyDevice };

    int r = crypt_setup::csInitEncrypt(phyPath, jobArgs.devName, &proc);
    if (r < 0) {
        qWarning() << EInitEnc + phyPath << r;
        job_file_helper::removeJobFile(jobArgs.jobFile);
        setExitCode(r);
        return;
    }
    qInfo() << "overlay encrypt initialized." + phyPath;

    r = crypt_setup::csActivateDeviceByVolume(phyPath, unlockName, proc.volumeKey);
    if (r < 0) {
        qWarning() << "cannot activate device by volume key, try using passphrase." + phyPath;
        RetOnFail(crypt_setup::csActivateDevice(phyPath, unlockName), EActive + phyPath);
    }
    qInfo() << "overlay device activated." + phyPath + unlockName;

    // reload midDev to unlocked dev. and resume topDev.
    auto unlockPath = "/dev/mapper/" + unlockName;
    dm_setup::DMTable reloadTab { "linear", unlockPath + " 0", 0, blockdev_helper::devBlockSize(unlockPath) };
    RetOnFail(dm_setup::dmReloadDevice(topName, reloadTab), EReload + topName);
    // RetOnFail(dm_setup::dmResumeDevice(midName), EResume + midName);
    // top dev is suspend in `detachPhyDevice`
    RetOnFail(dm_setup::dmResumeDevice(topName), EResume + topName);

    qInfo() << "overlay device encryption inited." << phyPath;
}

job_file_helper::JobDescArgs DMInitEncryptWorker::initJobArgs(const QString &phyDev, const QString &unlockName)
{
    job_file_helper::JobDescArgs args;
    auto ptr = blockdev_helper::createDevPtr(phyDev);
    if (!ptr) {
        qWarning() << "cannot create block for init job desc!" << phyDev;
        return args;
    }

    args.device = "PARTUUID=" + ptr->getProperty(dfmmount::Property::kPartitionUUID).toString();
    args.volume = unlockName;
    args.devName = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDeviceName).toString();
    args.devPath = phyDev;
    args.devType = disk_encrypt::job_type::TypeOverlay;

    return args;
}

int DMInitEncryptWorker::detachPhyDevice(int argc, const char *argv[])
{
    if (argc < 3) {
        qWarning() << "parameter error!" << argc << *argv;
        return -disk_encrypt::kErrorUnknown;
    }

    QString dev(argv[0]), topName(argv[1]), midName(argv[2]);
    auto devSize = blockdev_helper::devBlockSize(dev);
    dm_setup::DMTable initTab { "linear", dev + " 0", 0, devSize };
    dm_setup::DMTable zeroTab { "zero", "", 0, devSize };
    dm_setup::DMTable reloadTab { "linear", "/dev/mapper/" + midName + " 0", 0, devSize };

    qInfo() << "start suspend device" << topName;

    int r = disk_encrypt::kSuccess;
    do {
        // create midDev
        r = dm_setup::dmSuspendDevice(topName);
        if (r != 0) break;

        // physical device already detached.
        return disk_encrypt::kSuccess;
    } while (0);

    // dm_setup::dmRemoveDevice(midName);
    dm_setup::dmReloadDevice(topName, initTab);
    dm_setup::dmResumeDevice(topName);
    qWarning() << "dmsetup failed! reload to initial status." << topName;
    return r;
}
