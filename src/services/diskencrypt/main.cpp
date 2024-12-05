// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskencryptdbus.h"
#include "diskencryptadaptor.h"

#include <QCoreApplication>
#include <QDebug>

static constexpr char kObjPath[] { "/org/deepin/Filemanager/DiskEncrypt" };
static constexpr char kServiceName[] { "org.deepin.Filemanager.DiskEncrypt" };

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    DiskEncryptDBus encryptServer;
    new DiskEncryptAdaptor(&encryptServer);

    QDBusConnection connection = encryptServer.qDbusConnection();
    if (!connection.registerObject(kObjPath, &encryptServer)) {
        qWarning() << "failed to register dbus object" << connection.lastError().message();
    }
    if (!connection.registerService(kServiceName)) {
        qWarning() << "failed to register dbus object" << connection.lastError().message();
    }
    return a.exec();
}
