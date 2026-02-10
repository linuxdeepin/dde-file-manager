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

    std::vector<LaunchStrategy> strategies;

private:
    std::unique_ptr<QDBusInterface> dbusIface;
};

DFMBASE_END_NAMESPACE

#endif   // APPLAUNCHUTILS_P_H
