// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accesscontroldbus.h"
#include <DConfig>
#include <QDBusInterface>
#include <QPointer>
#include <QtConcurrent>

static AccessControlDBus *accessControlServer = nullptr;
static QPointer<Dtk::Core::DConfig> diskEncryptConfig;

static void activateDiskEncryptService()
{
    auto r = QtConcurrent::run([] {
        QDBusInterface iface("org.deepin.Filemanager.DiskEncrypt",
                             "/org/deepin/Filemanager/DiskEncrypt",
                             "org.deepin.Filemanager.DiskEncrypt",
                             QDBusConnection::systemBus());
        iface.call("IsTaskEmpty");
    });
    Q_UNUSED(r);
}

extern "C" int DSMRegister(const char *name, void *data)
{
    (void)data;
    accessControlServer = new AccessControlDBus(name);

    // deepin-service-manager 无法在启动时将加密服务拉起，即便加密服务设置了常驻
    // 但加密服务需要在启动时被拉起来，以便能够合并/更新 crypttab 文件
    // 在此处拉起有点脏但暂时没有更好的方法达到目的
    activateDiskEncryptService();

    // 监听 enableEncrypt 配置变更：当分区加密由关闭切换为开启时，
    // 磁盘加密服务因 main() 中的 enableEncrypt 门控而未运行，
    // 此处主动通过 DBus 激活拉起服务，使其建立 useOverlayDMMode 的 DConfig 监听
    diskEncryptConfig = Dtk::Core::DConfig::create("org.deepin.dde.file-manager",
                                                   "org.deepin.dde.file-manager.diskencrypt");
    if (diskEncryptConfig && diskEncryptConfig->isValid()) {
        QObject::connect(diskEncryptConfig, &Dtk::Core::DConfig::valueChanged,
                         diskEncryptConfig, [](const QString &key) {
            if (key != "enableEncrypt")
                return;
            if (diskEncryptConfig->value("enableEncrypt", false).toBool()) {
                qInfo() << "[AccessControl] enableEncrypt changed to true, activating disk encrypt service";
                activateDiskEncryptService();
            }
        });
    } else {
        qWarning() << "[AccessControl] failed to create valid DConfig for disk encrypt monitoring";
    }

    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    accessControlServer->deleteLater();
    accessControlServer = nullptr;
    if (diskEncryptConfig)
        diskEncryptConfig->deleteLater();
    return 0;
}
