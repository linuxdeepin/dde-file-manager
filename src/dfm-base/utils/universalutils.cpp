// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "universalutils.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>

#include <DDBusSender>

#include <QDir>
#include <QUrl>
#include <QCoreApplication>
#include <QApplication>
#include <QThread>
#include <QDebug>
#include <QX11Info>
#include <QFile>
#include <QProcess>
#include <QDBusConnectionInterface>
#include <QRegularExpression>

#ifdef COMPILE_ON_V23
#    define APP_MANAGER_SERVICE "org.deepin.dde.Application1.Manager"
#    define APP_MANAGER_PATH "/org/deepin/dde/Application1/Manager"
#    define APP_MANAGER_INTERFACE "org.deepin.dde.Application1.Manager"

#    define SYSTEM_SYSTEMINFO_SERVICE "org.deepin.dde.SystemInfo1"
#    define SYSTEM_SYSTEMINFO_PATH "/org/deepin/dde/SystemInfo1"
#    define SYSTEM_SYSTEMINFO_INTERFACE "org.deepin.dde.SystemInfo1"

#    define DEAMON_SYSTEMINFO_SERVICE "org.deepin.daemon.SystemInfo1"
#    define DEAMON_SYSTEMINFO_PATH "/org/deepin/daemon/SystemInfo1"
#    define DEAMON_SYSTEMINFO_INTERFACE "org.deepin.daemon.SystemInfo1"

#    define DEAMON_DOCK_SERVICE "org.deepin.dde.daemon.Dock1"
#    define DEAMON_DOCK_PATH "/org/deepin/dde/daemon/Dock1"
#    define DEAMON_DOCK_INTERFACE "org.deepin.dde.daemon.Dock1"

#    define DDE_LOCKSERVICE_SERVICE "org.deepin.dde.LockService1"
#    define DDE_LOCKSERVICE_PATH "/org/deepin/dde/LockService1"
#    define DDE_LOCKSERVICE_INTERFACE "org.deepin.dde.LockService1"
#else
#    define APP_MANAGER_SERVICE "com.deepin.SessionManager"
#    define APP_MANAGER_PATH "/com/deepin/StartManager"
#    define APP_MANAGER_INTERFACE "com.deepin.StartManager"

#    define SYSTEM_SYSTEMINFO_SERVICE "com.deepin.system.SystemInfo"
#    define SYSTEM_SYSTEMINFO_PATH "/com/deepin/system/SystemInfo"
#    define SYSTEM_SYSTEMINFO_INTERFACE "com.deepin.system.SystemInfo"

#    define DEAMON_SYSTEMINFO_SERVICE "com.deepin.daemon.SystemInfo"
#    define DEAMON_SYSTEMINFO_PATH "/com/deepin/daemon/SystemInfo"
#    define DEAMON_SYSTEMINFO_INTERFACE "com.deepin.daemon.SystemInfo"

#    define DEAMON_DOCK_SERVICE "com.deepin.dde.daemon.Dock"
#    define DEAMON_DOCK_PATH "/com/deepin/dde/daemon/Dock"
#    define DEAMON_DOCK_INTERFACE "com.deepin.dde.daemon.Dock"

#    define DDE_LOCKSERVICE_SERVICE "com.deepin.dde.LockService"
#    define DDE_LOCKSERVICE_PATH "/com/deepin/dde/LockService"
#    define DDE_LOCKSERVICE_INTERFACE "com.deepin.dde.LockService"
#endif

namespace dfmbase {

/*!
 * \brief send a messsage to Notification Center
 * \param msg
 */
void UniversalUtils::notifyMessage(const QString &msg)
{
    DDBusSender()
            .service("org.freedesktop.Notifications")
            .path("/org/freedesktop/Notifications")
            .interface("org.freedesktop.Notifications")
            .method(QString("Notify"))
            .arg(QObject::tr("dde-file-manager"))
            .arg(static_cast<uint>(0))
            .arg(QString("dde-file-manager"))
            .arg(msg)
            .arg(QString())
            .arg(QStringList())
            .arg(QVariantMap())
            .arg(5000)
            .call();
}

/*!
 * \brief send a messsage to Notification Center
 * \param title
 * \param msg
 */
void UniversalUtils::notifyMessage(const QString &title, const QString &msg)
{
    DDBusSender()
            .service("org.freedesktop.Notifications")
            .path("/org/freedesktop/Notifications")
            .interface("org.freedesktop.Notifications")
            .method(QString("Notify"))
            .arg(QObject::tr("dde-file-manager"))
            .arg(static_cast<uint>(0))
            .arg(QString("dde-file-manager"))
            .arg(title)
            .arg(msg)
            .arg(QStringList())
            .arg(QVariantMap())
            .arg(5000)
            .call();
}

/*!
 * \brief Determine if the current user is active or not
 * \return "State" property, we need it's "active" usunally
 */
QString UniversalUtils::userLoginState()
{
    QString state;
    QDBusInterface loginManager("org.freedesktop.login1",
                                "/org/freedesktop/login1/user/self",
                                "org.freedesktop.login1.User",
                                QDBusConnection::systemBus());
    QVariant replay = loginManager.property(("State"));
    if (replay.isValid())
        state = replay.toString();
    return state;
}

quint32 UniversalUtils::currentLoginUser()
{
    quint32 uid = UINT32_MAX;
    QDBusInterface loginManager("org.freedesktop.login1",
                                "/org/freedesktop/login1/user/self",
                                "org.freedesktop.login1.User",
                                QDBusConnection::systemBus());
    QVariant replay = loginManager.property(("UID"));
    if (replay.isValid())
        uid = replay.toULongLong();
    return uid;
}

bool UniversalUtils::isLogined()
{
    return userLoginState() == "active";
}

/*!
 * \brief UniversalUtils::inMainThread
 * \return true if in main thread
 */
bool UniversalUtils::inMainThread()
{
    return QThread::currentThread() == QCoreApplication::instance()->thread();
}

/*!
 * \brief FileUtils::blockShutdown 调用dbus去设置阻塞睡眠
 * \param replay 输入参数，dbus回复
 */
void UniversalUtils::blockShutdown(QDBusReply<QDBusUnixFileDescriptor> &replay)
{
    qInfo() << " create dbus to block computer shut down!!!";
    if (replay.value().isValid()) {
        qWarning() << "current qt dbus replyBlokShutDown is using!";
        return;
    }

    QDBusInterface loginManager("org.freedesktop.login1",
                                "/org/freedesktop/login1",
                                "org.freedesktop.login1.Manager",
                                QDBusConnection::systemBus());

    QList<QVariant> arg;
    arg << QString("shutdown:sleep:")   // what
        << qApp->applicationDisplayName()   // who
        << QObject::tr("Files are being processed")   // why
        << QString("block");   // mode

    replay = loginManager.callWithArgumentList(QDBus::Block, "Inhibit", arg);
    qInfo() << " create over dbus to block computer shut down!!!";
}

qint64 UniversalUtils::computerMemory()
{
    //! 从com.deepin.system.SystemInfo中获取实际安装的内存的大小
    QDBusInterface deepinSystemInfo(SYSTEM_SYSTEMINFO_SERVICE,
                                    SYSTEM_SYSTEMINFO_PATH,
                                    SYSTEM_SYSTEMINFO_INTERFACE,
                                    QDBusConnection::systemBus());
    // 部分数据优先从dbus读取
    // 获取安装的内存总量
    if (deepinSystemInfo.isValid())
        return static_cast<qint64>(deepinSystemInfo.property("MemorySize").toULongLong());

    return -1;
}

void UniversalUtils::computerInformation(QString &cpuinfo, QString &systemType, QString &edition, QString &version)
{
    QDBusInterface systemInfo(DEAMON_SYSTEMINFO_SERVICE,
                              DEAMON_SYSTEMINFO_PATH,
                              DEAMON_SYSTEMINFO_INTERFACE,
                              QDBusConnection::sessionBus());

    if (systemInfo.isValid()) {
        //! 获取cpu信息
        cpuinfo = qvariant_cast<QString>(systemInfo.property("Processor"));
        //! 获取系统是64位还是32位
        systemType = QString::number(qvariant_cast<qint64>(systemInfo.property("SystemType"))) + QObject::tr("Bit");

        if (edition.isEmpty()) {
            edition = qvariant_cast<QString>(systemInfo.property("Version"));
            QStringList temp = edition.split(' ');
            if (temp.size() > 1) {
                version = temp[0];
                edition = temp[1];
            } else if (!temp.isEmpty()) {
                edition = temp[0];
            }
        }
    }
}

double UniversalUtils::sizeFormat(qint64 size, QString &unit)
{
    static const double kSizeStep = 1024.0;
    static const QStringList kUnits { "B", "KB", "MB", "GB", "TB", "PB" };

    double formatedSize = size;
    int loopCount = 0;
    while (formatedSize >= kSizeStep && loopCount < kUnits.count() - 1) {
        formatedSize /= kSizeStep;
        loopCount += 1;
    }
    unit = kUnits[loopCount];
    return formatedSize;
}

/*!
 * \brief UniversalUtils::sizeFormat, returns the most appropriate size string. the return's number part is less than 1024.
 * \param size
 * \param percision, how many bit after dot.
 * \return
 */
QString UniversalUtils::sizeFormat(qint64 size, int percision)
{
    QString unit;
    double numberPart = sizeFormat(size, unit);
    return QString("%1 %2").arg(QString::number(numberPart, 'f', percision)).arg(unit);
}

bool UniversalUtils::checkLaunchAppInterface()
{
    static bool initStatus = true;
    static std::once_flag flag;
    std::call_once(flag, []() {
        QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
        if (!interface || !interface->isServiceRegistered(APP_MANAGER_SERVICE).value()) {
            initStatus = false;
            return;
        }
        QDBusInterface introspect(APP_MANAGER_SERVICE,
                                  APP_MANAGER_PATH,
                                  "org.freedesktop.DBus.Introspectable",
                                  QDBusConnection::sessionBus());
        introspect.setTimeout(1000);
        QDBusPendingReply<QString> reply = introspect.asyncCallWithArgumentList(QStringLiteral("Introspect"), {});
        reply.waitForFinished();
        if (reply.isFinished() && reply.isValid() && !reply.isError()) {
            QString xmlCode = reply.argumentAt(0).toString();
            if (xmlCode.contains(APP_MANAGER_INTERFACE)) {
                if (xmlCode.contains("LaunchApp")) {
                    initStatus = true;
                } else {
                    qWarning() << QString("%1 : doesn't have LaunchApp interface.").arg(APP_MANAGER_SERVICE);
                    initStatus = false;
                }
            } else {
                qWarning() << QString("%1 : Introspect error").arg(APP_MANAGER_SERVICE) << xmlCode;
                initStatus = false;
            }
        } else {
            initStatus = false;
        }
    });
    return initStatus;
}

bool UniversalUtils::launchAppByDBus(const QString &desktopFile, const QStringList &filePaths)
{
    QDBusInterface appManager(APP_MANAGER_SERVICE,
                              APP_MANAGER_PATH,
                              APP_MANAGER_INTERFACE,
                              QDBusConnection::sessionBus());

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(desktopFile) << QVariant::fromValue(static_cast<uint>(QX11Info::getTimestamp())) << QVariant::fromValue(filePaths);
    appManager.asyncCallWithArgumentList(QStringLiteral("LaunchApp"), argumentList);
    return true;
}

bool UniversalUtils::runCommand(const QString &cmd, const QStringList &args, const QString &wd)
{
    if (checkLaunchAppInterface()) {
        qDebug() << "launch cmd by dbus:" << cmd << args;
        QDBusInterface appManager(APP_MANAGER_SERVICE,
                                  APP_MANAGER_PATH,
                                  APP_MANAGER_INTERFACE,
                                  QDBusConnection::sessionBus());

        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(cmd) << QVariant::fromValue(args);

        if (!wd.isEmpty()) {
            QVariantMap opt = { { "dir", wd } };
            argumentList << QVariant::fromValue(opt);
            appManager.asyncCallWithArgumentList(QStringLiteral("RunCommandWithOptions"), argumentList);
        } else {
            appManager.asyncCallWithArgumentList(QStringLiteral("RunCommand"), argumentList);
        }

        return true;
    } else {
        qDebug() << "launch cmd by qt:" << cmd << args;
        return QProcess::startDetached(cmd, args, wd);
    }

    return false;
}

int UniversalUtils::dockHeight()
{
    QDBusInterface deepinDockInfo(DEAMON_DOCK_SERVICE,
                                  DEAMON_DOCK_PATH,
                                  DEAMON_DOCK_INTERFACE,
                                  QDBusConnection::sessionBus());

    int dockHeight = 0;
    if (deepinDockInfo.isValid()) {
        QVariant temp = deepinDockInfo.property("WindowSizeEfficient");
        dockHeight = temp.toInt();
    }
    return dockHeight;
}

QMap<QString, QString> UniversalUtils::getKernelParameters()
{
    QFile cmdline("/proc/cmdline");
    cmdline.open(QIODevice::ReadOnly);
    QByteArray content = cmdline.readAll();

    QByteArrayList paraList(content.split(' '));

    QMap<QString, QString> result;
    result.insert("_ori_proc_cmdline", content);

    for (const QByteArray &onePara : paraList) {
        int equalsIdx = onePara.indexOf('=');
        QString key = equalsIdx == -1 ? onePara.trimmed() : onePara.left(equalsIdx).trimmed();
        QString value = equalsIdx == -1 ? QString() : onePara.right(equalsIdx).trimmed();
        result.insert(key, value);
    }

    return result;
}

bool UniversalUtils::isInLiveSys()
{
    bool ret = false;
    static const QMap<QString, QString> &cmdline = getKernelParameters();
    if (cmdline.value("boot", "") == QStringLiteral("live"))
        ret = true;
    return ret;
}

QVariantHash UniversalUtils::convertFromQMap(const QVariantMap map)
{
    QVariantHash ret;
    auto iter = map.cbegin();
    while (iter != map.cend()) {
        ret.insert(iter.key(), iter.value());
        iter += 1;
    }
    return ret;
}

bool UniversalUtils::urlEquals(const QUrl &url1, const QUrl &url2)
{
    if (url1 == url2)
        return true;

    auto path1 { url1.path() }, path2 { url2.path() };
    if (!path1.endsWith("/"))
        path1.append("/");
    if (!path2.endsWith("/"))
        path2.append("/");

    if (url1.scheme() == url2.scheme() && path1 == path2 && url1.host() == url2.host())
        return true;
    return false;
}

bool UniversalUtils::urlsTransformToLocal(const QList<QUrl> &sourceUrls, QList<QUrl> *targetUrls)
{
    Q_ASSERT(targetUrls);
    bool ret { false };

    for (const auto &url : sourceUrls) {
        if (url.scheme() == Global::Scheme::kFile) {
            targetUrls->append(url);
            continue;
        }

        auto info { InfoFactory::create<FileInfo>(url) };
        if (info && info->canAttributes(FileInfo::FileCanType::kCanRedirectionFileUrl)) {
            ret = true;
            targetUrls->append(info->urlOf(UrlInfoType::kRedirectedFileUrl));
        } else {
            targetUrls->append(url);
        }
    }

    return ret;
}

bool UniversalUtils::urlTransformToLocal(const QUrl &sourceUrl, QUrl *targetUrls)
{
    Q_ASSERT(sourceUrl.isValid());
    Q_ASSERT(targetUrls);
    bool ret { false };

    if (sourceUrl.scheme() == Global::Scheme::kFile) {
        *targetUrls = sourceUrl;
        return ret;
    }

    auto info { InfoFactory::create<FileInfo>(sourceUrl) };
    if (info && info->canAttributes(FileInfo::FileCanType::kCanRedirectionFileUrl)) {
        ret = true;
        *targetUrls = info->urlOf(UrlInfoType::kRedirectedFileUrl);
    } else {
        *targetUrls = sourceUrl;
    }

    return ret;
}

QString UniversalUtils::getCurrentUser()
{
    QString user;

    QDBusInterface sessionManagerIface(DDE_LOCKSERVICE_SERVICE,
                                       DDE_LOCKSERVICE_PATH,
                                       DDE_LOCKSERVICE_INTERFACE,
                                       QDBusConnection::systemBus());

    if (sessionManagerIface.isValid()) {
        QDBusPendingCall call = sessionManagerIface.asyncCall("CurrentUser");
        call.waitForFinished();
        if (!call.isError()) {
            QDBusReply<QString> reply = call.reply();
            user = reply.value();
        }
    }

    return user;
}

void UniversalUtils::userChange(QObject *obj, const char *cslot)
{
    QDBusConnection::systemBus().connect(
            DDE_LOCKSERVICE_SERVICE,
            DDE_LOCKSERVICE_PATH,
            DDE_LOCKSERVICE_INTERFACE,
            "UserChanged",
            obj,
            cslot);
}

void UniversalUtils::prepareForSleep(QObject *obj, const char *cslot)
{
    QDBusConnection::systemBus().connect(
            "org.freedesktop.login1",
            "/org/freedesktop/login1",
            "org.freedesktop.login1.Manager",
            "PrepareForSleep",
            obj,
            cslot);
}

}
