// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systemservicemanager.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QProcess>
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

    // pkexec 会查找 systemctl 的完整路径，但显式指定更安全
    // 通常 systemctl 位于 /usr/bin/ 或 /bin/
    // QStandardPaths::findExecutable 可以帮助找到它
    QString program = "pkexec";
    QStringList arguments;
    arguments << "systemctl"
              << "enable"
              << "--now" << serviceName;

    qCInfo(logDFMBase) << "SystemServiceManager: Executing:" << program << arguments.join(" ");

    // 使用 QProcess::execute() 进行同步阻塞调用
    // 这会等待命令执行完成，并返回退出码
    int exitCode = QProcess::execute(program, arguments);

    // 分析执行结果
    bool success = (exitCode == 0);

    if (success) {
        qCInfo(logDFMBase) << "SystemServiceManager: Successfully enabled service" << serviceName;
    } else {
        // pkexec 的退出码有特殊含义：
        // 127: 命令未找到
        // 126: 用户取消了认证对话框
        // 其他非零值: 执行错误
        qCWarning(logDFMBase) << "SystemServiceManager: Failed to enable service" << serviceName
                              << "pkexec exited with code" << exitCode;
        if (exitCode == 126) {
            qCWarning(logDFMBase) << "SystemServiceManager: User cancelled the authentication dialog.";
        }
    }

    return success;
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

    // 使用 GetUnit 方法获取 unit 路径
    QDBusReply<QDBusObjectPath> reply = managerIface.call("GetUnit", serviceName);
    if (!reply.isValid()) {
        qCWarning(logDFMBase) << "SystemServiceManager: Failed to get unit path for" << serviceName
                              << "Error:" << reply.error().message();
        return QString();
    }

    return reply.value().path();
}
