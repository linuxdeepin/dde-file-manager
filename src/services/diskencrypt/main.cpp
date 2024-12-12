// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskencryptdbus.h"
#include "diskencryptadaptor.h"

#include <DConfig>

#include <QCoreApplication>
#include <QDebug>

static constexpr char kObjPath[] { "/org/deepin/Filemanager/DiskEncrypt" };
static constexpr char kServiceName[] { "org.deepin.Filemanager.DiskEncrypt" };

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    auto cfg = Dtk::Core::DConfig::create("org.deepin.dde.file-manager",
                                          "org.deepin.dde.file-manager.diskencrypt");
    bool enable = cfg->value("enableEncrypt", true).toBool();
    cfg->deleteLater();
    if (!enable) {
        qWarning() << "org.deepin.dde.file-manager.diskencrypt is dsiable, process exit";
        return 0;
    }

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
