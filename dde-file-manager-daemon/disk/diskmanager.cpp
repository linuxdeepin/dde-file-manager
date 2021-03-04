/*
* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
*
* Author: Liu Zhangjian<liuzhangjian@uniontech.com>
*
* Maintainer: Liu Zhangjian<liuzhangjian@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "diskmanager.h"
#include "app/policykithelper.h"
#include "dbusservice/dbusadaptor/disk_adaptor.h"

#include <QDBusConnection>
#include <QSettings>

#include <libcryptsetup.h>

enum VerifyInfo
{
    Success,
    ConfirmFailed,
    InitCryptFailed,
    DeviceIsEmpty,
    WrongPassword,
    CryptLoadFailed,
    Unknown
};

QString DiskManager::ObjectPath = "/com/deepin/filemanager/daemon/DiskManager";
QString DiskManager::PolicyKitActionId = "com.deepin.filemanager.daemon.DiskManager";

DiskManager::DiskManager(QObject *parent)
    : QObject(parent)
    , QDBusContext()
{
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_diskAdaptor = new DiskAdaptor(this);
}

DiskManager::~DiskManager()
{

}

bool DiskManager::checkAuthentication()
{
    bool ret = false;
    qint64 pid = 0;
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if (c.isConnected()) {
        pid = c.interface()->servicePid(message().service()).value();
    }

    if (pid) {
        ret = PolicyKitHelper::instance()->checkAuthorization(PolicyKitActionId, pid);
    }

    if (!ret) {
        qInfo() << "Authentication failed !!";
    }
    return ret;
}

int DiskManager::changePassword(const char *oldPwd, const char *newPwd, const char *device)
{
    struct crypt_device *cd = nullptr;
    int r;

    r = crypt_init(&cd, device);
    if (r < 0) {
        qInfo("crypt_init failed,code is:%d", r);
        return InitCryptFailed;
    }
    qInfo("Context is attached to block device %s.", crypt_get_device_name(cd));

    r = crypt_load(cd,		/* crypt context */
                   CRYPT_LUKS2,	/* requested type */
                   nullptr);		/* additional parameters (not used) */

    if (r < 0) {
        qInfo("crypt_load() failed on device %s.\n", crypt_get_device_name(cd));
        crypt_free(cd);
        return CryptLoadFailed;
    }

    r = crypt_activate_by_passphrase(cd, nullptr, CRYPT_ANY_SLOT,
                         oldPwd, strlen(oldPwd), CRYPT_ACTIVATE_ALLOW_UNBOUND_KEY);
    if (r < 0) {
        qInfo("crypt_activate_by_passphrase() failed on device %s.\n", crypt_get_device_name(cd));
        crypt_free(cd);
        return WrongPassword;
    }

    r = crypt_keyslot_change_by_passphrase(cd, CRYPT_ANY_SLOT,
                                           CRYPT_ANY_SLOT, oldPwd, strlen(oldPwd), newPwd, strlen(newPwd));
    qInfo("crypt_activate_by_passphrase() code:%d", r);
    crypt_free(cd);
    if (r < 0) {
        qInfo("crypt_init failed,code is:%d", r);
        return InitCryptFailed;
    }
    return Success;
}

void DiskManager::changeDiskPassword(const QString &oldPwd, const QString &newPwd)
{
    if (!checkAuthentication()) {
        emit confirmed(false);
        return;
    }

    emit confirmed(true);
    QSettings settings("/etc/deepin-installer.conf", QSettings::IniFormat);
    QString value = settings.value("DI_CRYPT_INFO").toString();
    QStringList devices = value.split(':');
    if (!devices.isEmpty()) {
        emit finished(changePassword(oldPwd.toLocal8Bit().data(), newPwd.toLocal8Bit().data(), devices[0].toLocal8Bit().data()));
        return;
    }

    emit finished(DeviceIsEmpty);
}
