// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "applaunchutils.h"
#include "applaunchutils_p.h"

#include <dfm-base/utils/universalutils.h>

#include <DUtil>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDebug>
#include <QUrl>

#undef signals
extern "C" {
#include <gio/gio.h>
#include <gio/gappinfo.h>
#include <gio-unix-2.0/gio/gdesktopappinfo.h>
}
#define signals public
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

bool AppLaunchUtilsPrivate::launchByDBus(const QString &desktopFile, const QStringList &filePaths)
{
    qCInfo(logDFMBase, "launch App By DBus, desktopFile : %s, files count : %d !", desktopFile.toStdString().c_str(), filePaths.count());
    qCDebug(logDFMBase) << "launch App By DBus, files : \n"
                        << filePaths;
    if (UniversalUtils::checkLaunchAppInterface())
        return UniversalUtils::launchAppByDBus(desktopFile, filePaths);
    return false;
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
                       return QUrl(urlStr).toLocalFile();
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
