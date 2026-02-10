// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accesscontroldbus.h"
#include <QDBusInterface>
#include <QtConcurrent>

static AccessControlDBus *accessControlServer = nullptr;

extern "C" int DSMRegister(const char *name, void *data)
{
    (void)data;
    accessControlServer = new AccessControlDBus(name);

    // deepin-service-manager 无法在启动时将加密服务拉起，即便加密服务设置了常驻
    // 但加密服务需要在启动时被拉起来，以便能够合并/更新 crypttab 文件
    // 在此处拉起有点脏但暂时没有更好的方法达到目的
    auto r = QtConcurrent::run([] {
        QDBusInterface iface("org.deepin.Filemanager.DiskEncrypt",
                             "/org/deepin/Filemanager/DiskEncrypt",
                             "org.deepin.Filemanager.DiskEncrypt",
                             QDBusConnection::systemBus());
        iface.call("IsTaskEmpty");
    });
    Q_UNUSED(r);

    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    accessControlServer->deleteLater();
    accessControlServer = nullptr;
    return 0;
}
