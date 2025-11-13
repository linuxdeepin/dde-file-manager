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
            qCritical() << msg << "error:" << r; \
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
    qInfo() << "==> DMDecryptWorker::run()";

    auto dev = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString();
    qInfo() << "About to decrypt overlay device:" << dev;

    auto fd = inhibit_helper::inhibit("Decrypting " + dev);

    auto passphrase = disk_encrypt::fromBase64(m_args.value(disk_encrypt::encrypt_param_keys::kKeyPassphrase).toString());
    auto displayName = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDeviceName).toString();
    qDebug() << "Display name:" << displayName;

    QString phyDev, clearDev;
    getDevPath(&phyDev, &clearDev);
    if (phyDev.isEmpty() || clearDev.isEmpty()) {
        qCritical() << "Associated device not found, device:" << dev;
        setExitCode(-disk_encrypt::kErrorUnknown);
        return;
    }
    qInfo() << "Physical device:" << phyDev << ", clear device:" << clearDev;

    QString usecName = blockdev_helper::getUSecName(dev);
    if (usecName.isEmpty()) {
        qCritical() << "Cannot find usec-overlay name from device:" << dev;
        setExitCode(-disk_encrypt::kErrorUnknown);
        return;
    }
    qDebug() << "USec overlay name:" << usecName;

    m_args.insert(disk_encrypt::encrypt_param_keys::kKeyDevice, phyDev);
    auto actName = clearDev.mid(sizeof("/dev/mapper/") - 1);
    qInfo() << "Starting decrypt process for physical device:" << phyDev << ", activation name:" << actName;

    QString detachHeader;
    crypt_setup_helper::genDetachHeaderPath(phyDev, &detachHeader);
    if (!detachHeader.isEmpty()) {
        crypttab_helper::addCryptOption(actName, "header="+detachHeader);
        qInfo() << "Crypttab updated, detach header set:" << detachHeader;
    }

    auto r = crypt_setup::csDecrypt(phyDev, passphrase, displayName, actName);
    if (r < 0) {
        qCritical() << "Decrypt failed, device:" << dev << "error:" << r;
        setExitCode(r);
        return;
    }
    qInfo() << "Decrypt completed successfully";

    // system("udevadm trigger");

    RetOnFail(dm_setup::dmSuspendDevice(usecName), ESuspend);
    qInfo() << "Device suspended:" << usecName;

    auto size = blockdev_helper::devBlockSize(phyDev);
    qDebug() << "Device block size:" << size;
    dm_setup::DMTable tab { "linear", phyDev + " 0", 0, size };
    r = dm_setup::dmReloadDevice(usecName, tab);
    if (r < 0) {
        qCritical() << "Reload device failed, device:" << usecName << "physical:" << phyDev << "size:" << size << "error:" << r;
        setExitCode(r);
        r = dm_setup::dmResumeDevice(usecName);
        qWarning() << "Device resumed after reload failure, device:" << usecName << "result:" << r;
        return;
    }
    qInfo() << "Device reloaded successfully:" << usecName;

    RetOnFail(dm_setup::dmResumeDevice(usecName), EResume);
    qInfo() << "Device resumed:" << usecName;

    // system("udevadm trigger");

    auto midName = usecName.replace("overlay", "overlay-mid");
    if (!QFile("/dev/mapper/" + midName).exists()) {
        qDebug() << "Mid device does not exist:" << midName;
    } else {
        r = dm_setup::dmRemoveDevice(midName);
        if (r < 0)
            qWarning() << "Cannot remove temp middle device, name:" << midName << "error:" << r;
        else
            qInfo() << "Temp middle device removed:" << midName;
    }

    qInfo() << "Overlay device decrypted successfully:" << phyDev;
    crypttab_helper::removeCryptItem(actName);
}

void DMDecryptWorker::getDevPath(QString *phyDev, QString *clearDev)
{
    Q_ASSERT(phyDev && clearDev);
    qDebug() << "==> DMDecryptWorker::getDevPath()";

    auto dev = m_args.value(disk_encrypt::encrypt_param_keys::kKeyDevice).toString();   // /dev/dm-?
    qDebug() << "Resolving device paths for:" << dev;

    *phyDev = dm_setup_helper::findHolderDev(dev);   // /dev/sd?
    if (phyDev->isEmpty()) {
        qCritical() << "Cannot find physical device for:" << dev;
        return;
    }
    qDebug() << "Physical device found:" << *phyDev;

    auto ptr = blockdev_helper::createDevPtr(*phyDev);
    if (!ptr) {
        qCritical() << "Cannot create device object, mapper device:" << dev << ", physical device:" << *phyDev;
        return;
    }

    *clearDev = ptr->getProperty(dfmmount::Property::kEncryptedCleartextDevice).toString();
    if (clearDev->isEmpty()) {
        qWarning() << "Device not unlocked or not encrypted, device:" << dev << ", physical:" << *phyDev;

        auto items = crypttab_helper::cryptItems();
        qDebug() << "Searching cleartext device in crypttab, items count:" << items.size();
        for (auto item: items) {
            bool matchPartUUID = item.source.startsWith("PARTUUID=")
                    && item.source.contains(ptr->getProperty(dfmmount::Property::kPartitionUUID).toString());
            bool matchUUID = item.source.startsWith("UUID=")
                    && item.source.contains(ptr->getProperty(dfmmount::Property::kBlockIDUUID).toString());
            bool matchDesc = item.source.startsWith("/dev/")
                    && item.source == *phyDev;
            if (matchPartUUID || matchUUID || matchDesc) {
                *clearDev = "/dev/mapper/" + item.target;
                qInfo() << "Found clear device in crypttab, physical:" << *phyDev << ", clear:" << *clearDev;
                return;
            }
        }

        return;
    }

    auto clearPtr = blockdev_helper::createDevPtr2(*clearDev);
    if (!clearPtr) {
        qCritical() << "Cannot create clear device object, mapper:" << dev << ", physical:" << *phyDev << ", clear:" << *clearDev;
        return;
    }
    auto name = clearPtr->getProperty(dfmmount::Property::kBlockPreferredDevice).toString();
    *clearDev = name;   // /dev/mapper/???
    qDebug() << "Clear device resolved:" << *clearDev;
}
