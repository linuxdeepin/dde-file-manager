// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "applaunchutils.h"
#include "applaunchutils_p.h"

#include <DUtil>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDebug>
#include <QUrl>

extern "C" {
#include <gio/gio.h>
#include <gio/gappinfo.h>
#include <gio-unix-2.0/gio/gdesktopappinfo.h>
}

DFMBASE_BEGIN_NAMESPACE

AppLaunchUtilsPrivate::AppLaunchUtilsPrivate()
{
    // Initialize default strategies
    strategies.push_back({
            std::bind(&AppLaunchUtilsPrivate::launchByDBus, this,
                      std::placeholders::_1, std::placeholders::_2),
            10   // DBus has higher priority
    });

    strategies.push_back({
            std::bind(&AppLaunchUtilsPrivate::launchByGio, this,
                      std::placeholders::_1, std::placeholders::_2),
            20   // GIO has lower priority
    });
}

AppLaunchUtilsPrivate::~AppLaunchUtilsPrivate() = default;

bool AppLaunchUtilsPrivate::checkDBusConnection() const
{
    return QDBusConnection::sessionBus().isConnected();
}

bool AppLaunchUtilsPrivate::checkLaunchAppInterface() const
{
    QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
    if (!interface || !interface->isServiceRegistered(DBusServiceNames::kService).value()) {
        qCWarning(logDFMBase) << "DBus service not registered:" << DBusServiceNames::kService;
        return false;
    }
    return true;
}

QDBusInterface *AppLaunchUtilsPrivate::dbusInterface(const QString &desktopFile)
{
    auto appId = DUtil::getAppIdFromAbsolutePath(desktopFile);
    if (appId.isEmpty()) {
        QFileInfo file { desktopFile };
        constexpr auto kDesktopSuffix { u8"desktop" };
        if (file.suffix() == kDesktopSuffix)
            appId = file.completeBaseName();
    }

    const auto &dbusAppId = DUtil::escapeToObjectPath(appId);
    const auto &objectPath = QString("%1/%2").arg(DBusServiceNames::kPathPrefix, dbusAppId);

    qCDebug(logDFMBase) << "Creating DBus interface for app:" << appId
                        << "object path:" << objectPath;

    dbusIface = std::make_unique<QDBusInterface>(
            DBusServiceNames::kService,
            objectPath,
            DBusServiceNames::kInterface,
            QDBusConnection::sessionBus());

    return dbusIface.get();
}
bool AppLaunchUtilsPrivate::launchByDBus(const QString &desktopFile, const QStringList &filePaths)
{
    qCInfo(logDFMBase) << "Attempting to launch application via DBus:"
                       << "\n  Desktop file:" << desktopFile
                       << "\n  Files count:" << filePaths.count();

    if (!checkLaunchAppInterface()) {
        qCWarning(logDFMBase) << "DBus launch interface not available";
        return false;
    }

    QDBusInterface *interface = dbusInterface(desktopFile);
    if (!interface || !interface->isValid()) {
        qCWarning(logDFMBase) << "Failed to create DBus interface";
        return false;
    }

    QVariantMap options;

    QDBusMessage reply = interface->callWithArgumentList(QDBus::Block,
                                                         "Launch",
                                                         { QString(), filePaths, options });

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCWarning(logDFMBase) << "DBus launch failed:" << reply.errorMessage();
        return false;
    }

    qCDebug(logDFMBase) << "Successfully launched application via DBus, launch: " << filePaths;
    return true;
}

bool AppLaunchUtilsPrivate::launchByGio(const QString &desktopFile, const QStringList &filePaths)
{
    qCInfo(logDFMBase) << "Attempting to launch application via GIO:"
                       << "\n  Desktop file:" << desktopFile
                       << "\n  Files:" << filePaths;

    const QByteArray &cDesktopFilePath = desktopFile.toLocal8Bit();
    g_autoptr(GDesktopAppInfo) appInfo = g_desktop_app_info_new_from_filename(cDesktopFilePath.data());

    if (!appInfo) {
        qCWarning(logDFMBase) << "Failed to create GDesktopAppInfo from desktop file";
        return false;
    }

    GList *gfiles = nullptr;
    for (const QString &path : filePaths) {
        QString url = QUrl::fromLocalFile(path).toString();
        const QByteArray &cFilePath = url.toLocal8Bit();
        GFile *gfile = g_file_new_for_uri(cFilePath.data());
        gfiles = g_list_append(gfiles, gfile);
    }

    g_autoptr(GError) gerror = nullptr;
    gboolean ok = g_app_info_launch(reinterpret_cast<GAppInfo *>(appInfo), gfiles, nullptr, &gerror);

    if (gerror) {
        qCWarning(logDFMBase) << "GIO launch error:" << gerror->message;
    }

    if (!ok) {
        qCWarning(logDFMBase) << "GIO launch failed";
    } else {
        qCDebug(logDFMBase) << "Successfully launched application via GIO";
    }

    if (gfiles)
        g_list_free(gfiles);

    return ok;
}

// Public interface implementation
AppLaunchUtils::AppLaunchUtils()
    : d(new AppLaunchUtilsPrivate())
{
}

AppLaunchUtils &AppLaunchUtils::instance()
{
    static AppLaunchUtils ins;
    return ins;
}

void AppLaunchUtils::addStrategy(AppLaunchFunc launcher, int priority)
{
    d->strategies.push_back({ launcher, priority });
    // Sort by priority (lower number = higher priority)
    std::sort(d->strategies.begin(), d->strategies.end(),
              [](const auto &a, const auto &b) { return a.priority < b.priority; });
}

bool AppLaunchUtils::launchApp(const QString &desktopFile, const QStringList &urlStrs)
{
    qCDebug(logDFMBase) << "Attempting to launch application with available strategies:"
                        << "\n  Desktop file:" << desktopFile
                        << "\n  Files:" << urlStrs;

    // NOTE: There may be performance issue
    QStringList localPaths;
    localPaths.reserve(urlStrs.size());
    std::transform(urlStrs.begin(), urlStrs.end(), std::back_inserter(localPaths),
                   [](const QString &urlStr) -> QString {
                       auto url = QUrl(urlStr);
                       auto localPath = url.isLocalFile() ? QUrl(urlStr).toLocalFile() : urlStr;
                       return localPath;
                   });

    for (const auto &strategy : d->strategies) {
        if (strategy.launcher(desktopFile, localPaths)) {
            qCDebug(logDFMBase) << "Successfully launched application";
            return true;
        }
    }

    qCWarning(logDFMBase) << "All launch strategies failed";
    return false;
}

bool AppLaunchUtils::defaultLaunchApp(const QString &desktopFile, const QStringList &paths)
{
    qCDebug(logDFMBase) << "Attempting to launch application with default:"
                        << "\n  Desktop file:" << desktopFile
                        << "\n  Files:" << paths;

    // 避免自定义策略陷入死循环
    if (d->launchByDBus(desktopFile, paths))
        return true;

    if (d->launchByGio(desktopFile, paths))
        return true;

    return false;
}

DFMBASE_END_NAMESPACE
