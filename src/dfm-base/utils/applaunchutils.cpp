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
#include <QDBusMetaType>   // qDBusRegisterMetaType
#include <QMetaType>   // Q_DECLARE_METATYPE
#include <QRegularExpression>

extern "C" {
#include <gio/gio.h>
#include <gio/gappinfo.h>
#include <gio-unix-2.0/gio/gdesktopappinfo.h>
}

// a{ss} 字典的 D-Bus marshalling。必须在全局作用域注册：
// QMetaTypeId 模板由 Qt 在全局命名空间声明，跨命名空间特化会被编译器拒绝。
// dfmbase::QStringMap 是 QMap<QString,QString> 的 using 别名（见 applaunchutils.h），
// 无逗号单 token，既绕过宏参数分隔，又与 AM1 introspection 的 QtTypeName.In4 一致。
Q_DECLARE_METATYPE(dfmbase::QStringMap)

DFMBASE_BEGIN_NAMESPACE

// Qt6 内置了 QMap<QString,QString> → a{ss} 的 D-Bus marshalling，
// 只需注册类型即可，无需手写 operator<< / operator>>。
namespace {
void ensureAM1MetaTypeRegistered()
{
    static const auto kRegistered = []() {
        return qDBusRegisterMetaType<QStringMap>();
    }();
    Q_UNUSED(kRegistered)
}
}   // namespace

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

    QVariantMap options { { QStringLiteral("_launch_type"), QStringLiteral("dde-file-manager") } };

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

bool AppLaunchUtilsPrivate::executeCommand(const QString &program, const QStringList &args,
                                           const QString &type, const QString &workdir,
                                           const QString &runId, const QStringMap &envVars)
{
    ensureAM1MetaTypeRegistered();

    qCDebug(logDFMBase) << "AM1 executeCommand:"
                        << "\n  program:" << program
                        << "\n  args:" << args
                        << "\n  type:" << type
                        << "\n  workdir:" << workdir;

    if (!checkLaunchAppInterface()) {
        qCWarning(logDFMBase) << "AM1 service not registered";
        return false;
    }

    QDBusInterface iface(DBusServiceNames::kService,
                         DBusServiceNames::kPathPrefix,   // /org/desktopspec/ApplicationManager1
                         DBusServiceNames::kManagerInterface,
                         QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        qCWarning(logDFMBase) << "AM1 manager interface invalid:" << iface.lastError().message();
        return false;
    }

    // 同步 D-Bus 调用默认超时 25s，若 AM1 挂起会阻塞 UI 线程。
    // executeCommand 只需创建 systemd transient unit，正常 <1s 返回。
    // 设 5s 超时：足够容错，又远小于默认值，超时即返回 false 降级到 QProcess。
    iface.setTimeout(5000);

    QDBusReply<QDBusObjectPath> reply = iface.call(QStringLiteral("executeCommand"),
                                                   program, args, type, runId,
                                                   QVariant::fromValue(envVars), workdir);
    if (!reply.isValid()) {
        qCWarning(logDFMBase) << "executeCommand failed:" << reply.error().message();
        return false;
    }

    qCDebug(logDFMBase) << "executeCommand succeeded, instance:" << reply.value().path();
    return true;
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

bool AppLaunchUtils::executeCommand(const QString &program, const QStringList &args,
                                    const QString &type, const QString &workdir,
                                    const QString &runId, const QStringMap &envVars)
{
    return d->executeCommand(program, args, type, workdir, runId, envVars);
}

DFMBASE_END_NAMESPACE
