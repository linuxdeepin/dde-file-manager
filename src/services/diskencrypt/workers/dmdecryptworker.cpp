// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
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
            qCritical() << "[DMDecryptWorker::run]" << msg << "error:" << r; \
            return;            \
        }                      \
    }

#define ESuspend "Error when suspending dm device"
#define EResume "Error when resuming dm device"
#define EReload "Error when reloading dm device"

FILE_ENCRYPT_USE_NS

DMDecryptWorker::DMDecryptWorker(const QVariantMap &args, QObject *parent)
    : BaseEncryptWorker(args, parent)
{
}

void DMDecryptWorker::run()
{
    qInfo() << "[DMDecryptWorker::run] Starting DM overlay decryption";

    auto dev = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString();
    qInfo() << "[DMDecryptWorker::run] About to decrypt overlay device:" << dev;

    auto fd = inhibit_helper::inhibit("Decrypting " + dev);

    auto passphrase = disk_encrypt::fromBase64(m_args.value(disk_encrypt::encrypt_param_keys::kKeyPassphrase).toString());
    auto displayName = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDeviceName).toString();
    qDebug() << "[DMDecryptWorker::run] Display name:" << displayName;

    QString phyDev, clearDev;
    getDevPath(&phyDev, &clearDev);
    if (phyDev.isEmpty() || clearDev.isEmpty()) {
        qCritical() << "[DMDecryptWorker::run] Associated device not found, device:" << dev;
        setExitCode(-disk_encrypt::kErrorUnknown);
        return;
    }
    qInfo() << "[DMDecryptWorker::run] Physical device:" << phyDev << ", clear device:" << clearDev;

    QString usecName = blockdev_helper::getUSecName(dev);
    if (usecName.isEmpty()) {
        qCritical() << "[DMDecryptWorker::run] Cannot find usec-overlay name from device:" << dev;
        setExitCode(-disk_encrypt::kErrorUnknown);
        return;
    }
    qDebug() << "[DMDecryptWorker::run] USec overlay name:" << usecName;

    m_args.insert(disk_encrypt::encrypt_param_keys::kKeyDevice, phyDev);
    auto actName = clearDev.mid(sizeof("/dev/mapper/") - 1);
    qInfo() << "[DMDecryptWorker::run] Starting decrypt process for physical device:" << phyDev << ", activation name:" << actName;

    QString detachHeader;
    crypt_setup_helper::genDetachHeaderPath(phyDev, &detachHeader);
    if (!detachHeader.isEmpty()) {
        crypttab_helper::addCryptOption(actName, "header="+detachHeader);
        qInfo() << "[DMDecryptWorker::run] Crypttab updated, detach header set:" << detachHeader;
    }

    auto r = crypt_setup::csDecrypt(phyDev, passphrase, displayName, actName);
    if (r < 0) {
        qCritical() << "[DMDecryptWorker::run] Decrypt failed, device:" << dev << "error:" << r;
        setExitCode(r);
        return;
    }
    qInfo() << "[DMDecryptWorker::run] Decrypt completed successfully";

    // system("udevadm trigger");

    RetOnFail(dm_setup::dmSuspendDevice(usecName), ESuspend);
    qInfo() << "[DMDecryptWorker::run] Device suspended:" << usecName;

    auto size = blockdev_helper::devBlockSize(phyDev);
    qDebug() << "[DMDecryptWorker::run] Device block size:" << size;
    dm_setup::DMTable tab { "linear", phyDev + " 0", 0, size };
    r = dm_setup::dmReloadDevice(usecName, tab);
    if (r < 0) {
        qCritical() << "[DMDecryptWorker::run] Reload device failed, device:" << usecName << "physical:" << phyDev << "size:" << size << "error:" << r;
        setExitCode(r);
        r = dm_setup::dmResumeDevice(usecName);
        qWarning() << "[DMDecryptWorker::run] Device resumed after reload failure, device:" << usecName << "result:" << r;
        return;
    }
    qInfo() << "[DMDecryptWorker::run] Device reloaded successfully:" << usecName;

    RetOnFail(dm_setup::dmResumeDevice(usecName), EResume);
    qInfo() << "[DMDecryptWorker::run] Device resumed:" << usecName;

    // system("udevadm trigger");

    auto midName = usecName.replace("overlay", "overlay-mid");
    if (!QFile("/dev/mapper/" + midName).exists()) {
        qDebug() << "[DMDecryptWorker::run] Mid device does not exist:" << midName;
    } else {
        r = dm_setup::dmRemoveDevice(midName);
        if (r < 0)
            qWarning() << "[DMDecryptWorker::run] Cannot remove temp middle device, name:" << midName << "error:" << r;
        else
            qInfo() << "[DMDecryptWorker::run] Temp middle device removed:" << midName;
    }

    qInfo() << "[DMDecryptWorker::run] Overlay device decrypted successfully:" << phyDev;
    crypttab_helper::removeCryptItem(actName);
}

void DMDecryptWorker::getDevPath(QString *phyDev, QString *clearDev)
{
    Q_ASSERT(phyDev && clearDev);
    qDebug() << "[DMDecryptWorker::getDevPath] Getting device paths";

    auto dev = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString();   // /dev/dm-?
    qDebug() << "[DMDecryptWorker::getDevPath] Resolving device paths for:" << dev;

    *phyDev = dm_setup_helper::findHolderDev(dev);   // /dev/sd?
    if (phyDev->isEmpty()) {
        qCritical() << "[DMDecryptWorker::getDevPath] Cannot find physical device for:" << dev;
        return;
    }
    qDebug() << "[DMDecryptWorker::getDevPath] Physical device found:" << *phyDev;

    auto ptr = blockdev_helper::createDevPtr(*phyDev);
    if (!ptr) {
        qCritical() << "[DMDecryptWorker::getDevPath] Cannot create device object, mapper device:" << dev << ", physical device:" << *phyDev;
        return;
    }

    *clearDev = ptr->getProperty(dfmmount::Property::kEncryptedCleartextDevice).toString();
    if (clearDev->isEmpty()) {
        qWarning() << "[DMDecryptWorker::getDevPath] Device not unlocked or not encrypted, device:" << dev << ", physical:" << *phyDev;

        auto items = crypttab_helper::cryptItems();
        qDebug() << "[DMDecryptWorker::getDevPath] Searching cleartext device in crypttab, items count:" << items.size();
        for (auto item: items) {
            bool matchPartUUID = item.source.startsWith("PARTUUID=")
                    && item.source.contains(ptr->getProperty(dfmmount::Property::kPartitionUUID).toString());
            bool matchUUID = item.source.startsWith("UUID=")
                    && item.source.contains(ptr->getProperty(dfmmount::Property::kBlockIDUUID).toString());
            bool matchDesc = item.source.startsWith("/dev/")
                    && item.source == *phyDev;
            if (matchPartUUID || matchUUID || matchDesc) {
                *clearDev = "/dev/mapper/" + item.target;
                qInfo() << "[DMDecryptWorker::getDevPath] Found clear device in crypttab, physical:" << *phyDev << ", clear:" << *clearDev;
                return;
            }
        }

        return;
    }

    auto clearPtr = blockdev_helper::createDevPtr2(*clearDev);
    if (!clearPtr) {
        qCritical() << "[DMDecryptWorker::getDevPath] Cannot create clear device object, mapper:" << dev << ", physical:" << *phyDev << ", clear:" << *clearDev;
        return;
    }
    auto name = clearPtr->getProperty(dfmmount::Property::kBlockPreferredDevice).toString();
    *clearDev = name;   // /dev/mapper/???
    qDebug() << "[DMDecryptWorker::getDevPath] Clear device resolved:" << *clearDev;
}
