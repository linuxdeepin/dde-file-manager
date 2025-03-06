// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dmdecryptworker.h"

#include "core/dmsetup.h"
#include "core/cryptsetup.h"
#include "helpers/blockdevhelper.h"
#include "helpers/inhibithelper.h"
#include "helpers/crypttabhelper.h"

#include <QFile>

#define RetOnFail(ret, msg)    \
    {                          \
        int r = ret;           \
        if (r < 0) {           \
            setExitCode(r);    \
            qWarning() << msg; \
            return;            \
        }                      \
    }

#define ESuspend "error when SUSPEND dm device"
#define EResume "error when RESUME dm device"
#define EReload "error when RELOAD dm device"

FILE_ENCRYPT_USE_NS

DMDecryptWorker::DMDecryptWorker(const QVariantMap &args, QObject *parent)
    : BaseEncryptWorker(args, parent)
{
}

void DMDecryptWorker::run()
{
    qInfo() << "about to decrypt overlay device...";

    auto dev = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString();
    auto fd = inhibit_helper::inhibit("Decrypting " + dev);

    auto passphrase = disk_encrypt::fromBase64(m_args.value(disk_encrypt::encrypt_param_keys::kKeyPassphrase).toString());
    auto displayName = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDeviceName).toString();
    QString phyDev, clearDev;
    getDevPath(&phyDev, &clearDev);
    if (phyDev.isEmpty() || clearDev.isEmpty()) {
        qWarning() << "associated device not found." << dev;
        setExitCode(-disk_encrypt::kErrorUnknown);
        return;
    }

    QString usecName = blockdev_helper::getUSecName(dev);
    if (usecName.isEmpty()) {
        qWarning() << "cannot find usec-overlay name from" << dev;
        setExitCode(-disk_encrypt::kErrorUnknown);
        return;
    }

    m_args.insert(disk_encrypt::encrypt_param_keys::kKeyDevice, phyDev);
    qInfo() << "about to decrypt device" << phyDev;
    auto actName = clearDev.mid(sizeof("/dev/mapper/") - 1);

    QString detachHeader;
    crypt_setup_helper::genDetachHeaderPath(phyDev, &detachHeader);
    if (!detachHeader.isEmpty()) {
        crypttab_helper::addCryptOption(actName, "header="+detachHeader);
        qInfo() << "crypttab updated, detach header setted.";
    }

    auto r = crypt_setup::csDecrypt(phyDev, passphrase, displayName, actName);
    if (r < 0) {
        qWarning() << "decrypt failed!" << dev << r;
        setExitCode(r);
        return;
    }

    // system("udevadm trigger");

    RetOnFail(dm_setup::dmSuspendDevice(usecName), ESuspend);
    qInfo() << usecName << "suspended.";

    auto size = blockdev_helper::devBlockSize(phyDev);
    dm_setup::DMTable tab { "linear", phyDev + " 0", 0, size };
    r = dm_setup::dmReloadDevice(usecName, tab);
    if (r < 0) {
        qWarning() << "reload device failed!" << usecName << phyDev << size;
        setExitCode(r);
        r = dm_setup::dmResumeDevice(usecName);
        qWarning() << "device resumed." << usecName << r;
        return;
    }
    qInfo() << usecName << "reloaded." << phyDev << size;

    RetOnFail(dm_setup::dmResumeDevice(usecName), EResume);
    qInfo() << usecName << "resumed.";

    // system("udevadm trigger");

    auto midName = usecName.replace("overlay", "overlay-mid");
    if (!QFile("/dev/mapper/" + midName).exists()) {
        qInfo() << midName << "do not exist.";
    } else {
        r = dm_setup::dmRemoveDevice(midName);
        if (r < 0)
            qWarning() << "cannot remove temp middle device!" << midName;
        else
            qInfo() << midName << "removed.";
    }

    qInfo() << "overlay device decrypted." << phyDev;
    crypttab_helper::removeCryptItem(actName);
}

void DMDecryptWorker::getDevPath(QString *phyDev, QString *clearDev)
{
    Q_ASSERT(phyDev && clearDev);

    auto dev = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString();   // /dev/dm-?
    *phyDev = dm_setup_helper::findHolderDev(dev);   // /dev/sd?
    if (phyDev->isEmpty()) {
        qWarning() << "cannot find physical device!" << dev;
        return;
    }

    auto ptr = blockdev_helper::createDevPtr(*phyDev);
    if (!ptr) {
        qWarning() << "cannot create device object!" << dev << *phyDev;
        return;
    }

    *clearDev = ptr->getProperty(dfmmount::Property::kEncryptedCleartextDevice).toString();
    if (clearDev->isEmpty()) {
        qWarning() << "device not unlock or not encrypt." << dev << phyDev;
        return;
    }

    auto clearPtr = blockdev_helper::createDevPtr2(*clearDev);
    if (!clearPtr) {
        qWarning() << "cannot create clear device object!" << dev << phyDev << clearDev;
        return;
    }
    auto name = clearPtr->getProperty(dfmmount::Property::kBlockPreferredDevice).toString();
    *clearDev = name;   // /dev/mapper/???
}
