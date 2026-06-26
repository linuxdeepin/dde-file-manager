// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLAUNCHUTILS_P_H
#define APPLAUNCHUTILS_P_H

#include "applaunchutils.h"

#include <QDBusInterface>
#include <vector>

DFMBASE_BEGIN_NAMESPACE

// DBus service names for V2X
struct DBusServiceNames
{
    static constexpr const char *kService = "org.desktopspec.ApplicationManager1";
    static constexpr const char *kPathPrefix = "/org/desktopspec/ApplicationManager1";
    static constexpr const char *kInterface = "org.desktopspec.ApplicationManager1.Application";
    // 管理器接口（executeCommand 在此接口上，与 per-app Application 接口不同）
    static constexpr const char *kManagerInterface = "org.desktopspec.ApplicationManager1";
};

struct LaunchStrategy
{
    AppLaunchFunc launcher;
    int priority;
};

class AppLaunchUtilsPrivate
{
public:
    AppLaunchUtilsPrivate();
    ~AppLaunchUtilsPrivate();

    // DBus related
    bool checkDBusConnection() const;
    bool checkLaunchAppInterface() const;
    QDBusInterface *dbusInterface(const QString &desktopFile);
    bool launchByDBus(const QString &desktopFile, const QStringList &filePaths);

    // GIO related
    bool launchByGio(const QString &desktopFile, const QStringList &filePaths);

    // AM1 管理器接口：executeCommand（用于无 desktop 文件的可执行程序/脚本）
    bool executeCommand(const QString &program, const QStringList &args,
                        const QString &type, const QString &workdir,
                        const QString &runId, const QStringMap &envVars);

    std::vector<LaunchStrategy> strategies;

private:
    std::unique_ptr<QDBusInterface> dbusIface;
};

DFMBASE_END_NAMESPACE

#endif   // APPLAUNCHUTILS_P_H
