// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskmanager.h"
#include "app/policykithelper.h"
#include "dbusadaptor/disk_adaptor.h"

#include <QDBusConnection>
#include <QSettings>
#include <QDBusConnectionInterface>
#include <QDebug>

#include <libcryptsetup.h>

QString DiskManager::ObjectPath = "/com/deepin/filemanager/daemon/DiskManager";
QString DiskManager::PolicyKitActionId = "com.deepin.filemanager.daemon.DiskManager";

DiskManager::DiskManager(QObject *parent)
    : QObject(parent)
{
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_diskAdaptor = new DiskAdaptor(this);
}

bool DiskManager::checkAuthentication()
{
    bool ret = false;
    qint64 pid = 0;
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if (c.isConnected())
        pid = c.interface()->servicePid(message().service()).value();

    if (pid)
        ret = PolicyKitHelper::instance()->checkAuthorization(PolicyKitActionId, pid);

    if (!ret)
        qInfo() << "Authentication failed !!";

    return ret;
}

void DiskManager::changeDiskPassword(const QString &oldPwd, const QString &newPwd)
{
    QStringList deviceList = encryptedDisks();
    if (deviceList.isEmpty()) {
        emit passwordChecked(true);
        QTimer::singleShot(500, [this] { emit finished(AccessDiskFailed); });
        return;
    }

    QByteArray decodedByteArray = oldPwd.toUtf8();
    const QByteArray &tmpOldPwd = QByteArray::fromBase64(decodedByteArray);

    decodedByteArray = oldPwd.toUtf8();
    const QByteArray &tmpNewPwd = QByteArray::fromBase64(decodedByteArray);

    int ret = Unknown;
    QStringList successList;
    for (int i = 0; i < deviceList.size(); ++i) {
        ret = changePassword(tmpOldPwd.data(), tmpNewPwd.data(), deviceList[i].toLocal8Bit().data());

        if (i == 0) {
            if (ret == PasswordWrong) {
                emit passwordChecked(false);
                return;
            }
        } else if (ret == PasswordWrong) {
            ret = PasswordInconsistent;
            break;
        }

        if (ret != Success)
            break;

        successList << deviceList[i];
    }

    // restore password
    if (ret != Success && !successList.isEmpty()) {
        m_isRestore = true;
        for (const auto &device : successList) {
            changePassword(tmpNewPwd.data(), tmpOldPwd.data(), device.toLocal8Bit().data());
        }
    }

    emit finished(ret);
}

int DiskManager::changePassword(const char *oldPwd, const char *newPwd, const char *device)
{
    struct crypt_device *cd = nullptr;
    int r;

    r = crypt_init(&cd, device);
    if (r < 0) {
        qInfo("crypt_init failed,code is:%d", r);
        return InitFailed;
    }
    qInfo("Context is attached to block device %s.", crypt_get_device_name(cd));

    r = crypt_load(cd, /* crypt context */
                   CRYPT_LUKS2, /* requested type */
                   nullptr); /* additional parameters (not used) */

    if (r < 0) {
        qInfo("crypt_load() failed on device %s.\n", crypt_get_device_name(cd));
        crypt_free(cd);
        return InitFailed;
    }

    r = crypt_activate_by_passphrase(cd, nullptr, CRYPT_ANY_SLOT,
                                     oldPwd, strlen(oldPwd), CRYPT_ACTIVATE_ALLOW_UNBOUND_KEY);
    if (r < 0) {
        qInfo("crypt_activate_by_passphrase() failed on device %s.\n", crypt_get_device_name(cd));
        crypt_free(cd);
        return PasswordWrong;
    } else if (!m_isRestore && encryptedDisks().first().compare(device) == 0) {
        emit passwordChecked(true);
    }

    r = crypt_keyslot_change_by_passphrase(cd, CRYPT_ANY_SLOT,
                                           CRYPT_ANY_SLOT, oldPwd, strlen(oldPwd), newPwd, strlen(newPwd));
    qInfo("crypt_activate_by_passphrase() code:%d", r);
    crypt_free(cd);
    if (r < 0) {
        qInfo("crypt_init failed,code is:%d", r);
        return InitFailed;
    }

    return Success;
}

QStringList DiskManager::encryptedDisks()
{
    QStringList deviceList;
    QSettings settings("/etc/deepin-installer.conf", QSettings::IniFormat);
    QString value = settings.value("DI_CRYPT_INFO").toString();
    if (!value.isEmpty()) {
        QStringList groupList = value.split(';');
        for (const auto &group : groupList) {
            QStringList device = group.split(':');
            if (!device.isEmpty())
                deviceList << device.first();
        }
    }

    return deviceList;
}
