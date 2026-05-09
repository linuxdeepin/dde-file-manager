// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systemservicemanager.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDebug>

using namespace dfmbase;

// systemd D-Bus 服务相关常量
static constexpr char kSystemdService[] { "org.freedesktop.systemd1" };
static constexpr char kSystemdManagerPath[] { "/org/freedesktop/systemd1" };
static constexpr char kSystemdManagerInterface[] { "org.freedesktop.systemd1.Manager" };
static constexpr char kSystemdUnitInterface[] { "org.freedesktop.systemd1.Unit" };

SystemServiceManager &SystemServiceManager::instance()
{
    static SystemServiceManager ins;
    return ins;
}

SystemServiceManager::SystemServiceManager(QObject *parent)
    : QObject(parent)
{
}

bool SystemServiceManager::isServiceRunning(const QString &serviceName)
{
    if (serviceName.isEmpty()) {
        qCWarning(logDFMBase) << "SystemServiceManager: Service name is empty";
        return false;
    }

    QString unitPath = unitPathFromName(serviceName);
    if (unitPath.isEmpty()) {
        qCWarning(logDFMBase) << "SystemServiceManager: Failed to get unit path for" << serviceName;
        return false;
    }

    QDBusInterface unitIface(
            kSystemdService,
            unitPath,
            kSystemdUnitInterface,
            QDBusConnection::systemBus());

    if (!unitIface.isValid()) {
        qCWarning(logDFMBase) << "SystemServiceManager: Invalid D-Bus interface for" << serviceName
                              << "Error:" << unitIface.lastError().message();
        return false;
    }

    QVariant state = unitIface.property("SubState");
    if (!state.isValid()) {
        qCWarning(logDFMBase) << "SystemServiceManager: Failed to get SubState for" << serviceName;
        return false;
    }

    return state.toString() == "running";
}

bool SystemServiceManager::serviceExists(const QString &serviceName)
{
    return !unitPathFromName(serviceName).isEmpty();
}

bool SystemServiceManager::startService(const QString &serviceName)
{
    if (serviceName.isEmpty()) {
        qCWarning(logDFMBase) << "SystemServiceManager: Service name is empty";
        return false;
    }

    QDBusInterface managerIface(
            kSystemdService,
            kSystemdManagerPath,
            kSystemdManagerInterface,
            QDBusConnection::systemBus());

    if (!managerIface.isValid()) {
        qCWarning(logDFMBase) << "SystemServiceManager: Invalid Manager D-Bus interface"
                              << "Error:" << managerIface.lastError().message();
        return false;
    }

    // StartUnit 方法会自动通过 PolicyKit 进行鉴权
    // 参数说明：
    // - serviceName: 服务名称
    // - "replace": 替换模式，如果已有挂起的启动任务则替换
    QDBusReply<QDBusObjectPath> reply = managerIface.call(
            "StartUnit",
            serviceName,
            "replace");

    bool success = reply.isValid();
    if (!success) {
        qCWarning(logDFMBase) << "SystemServiceManager: Failed to start service" << serviceName
                              << "Error:" << reply.error().message();
    } else {
        qCInfo(logDFMBase) << "SystemServiceManager: Successfully started service" << serviceName;
    }

    return success;
}

bool SystemServiceManager::enableServiceNow(const QString &serviceName)
{
    if (serviceName.isEmpty()) {
        qCWarning(logDFMBase) << "SystemServiceManager: Service name is empty";
        return false;
    }

    // 通过 systemd D-Bus 接口 EnableUnitFiles 启用服务开机自启
    // 使用 setInteractiveAuthorizationAllowed(true) 让 polkit 弹出认证对话框，
    // 走 D-Bus + polkit 鉴权路径，避免 pkexec 白名单限制
    QDBusMessage msg = QDBusMessage::createMethodCall(
            kSystemdService,
            kSystemdManagerPath,
            kSystemdManagerInterface,
            "EnableUnitFiles");
    msg.setInteractiveAuthorizationAllowed(true);

    // EnableUnitFiles(in as unitNames, in b runtime, in b reload)
    // runtime=false: 持久化到磁盘; reload=true: 自动 ReloadDaemon
    msg << QStringList{ serviceName } << false << true;

    QDBusMessage reply = QDBusConnection::systemBus().call(msg, QDBus::Block);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCWarning(logDFMBase) << "SystemServiceManager: EnableUnitFiles failed for" << serviceName
                              << "Error:" << reply.errorMessage();
        return false;
    }

    qCInfo(logDFMBase) << "SystemServiceManager: Successfully enabled service" << serviceName;

    // 启动服务（复用已有的 D-Bus StartUnit 实现）
    return startService(serviceName);
}

QString SystemServiceManager::unitPathFromName(const QString &serviceName)
{
    if (serviceName.isEmpty()) {
        return QString();
    }

    QDBusInterface managerIface(
            kSystemdService,
            kSystemdManagerPath,
            kSystemdManagerInterface,
            QDBusConnection::systemBus());

    if (!managerIface.isValid()) {
        qCWarning(logDFMBase) << "SystemServiceManager: Invalid Manager D-Bus interface"
                              << "Error:" << managerIface.lastError().message();
        return QString();
    }

    // LoadUnit can resolve installed but currently inactive units; it fails when
    // the unit file is unavailable.
    QDBusReply<QDBusObjectPath> reply = managerIface.call("LoadUnit", serviceName);
    if (!reply.isValid()) {
        qCWarning(logDFMBase) << "SystemServiceManager: Failed to get unit path for" << serviceName
                              << "Error:" << reply.error().message();
        return QString();
    }

    return reply.value().path();
}
