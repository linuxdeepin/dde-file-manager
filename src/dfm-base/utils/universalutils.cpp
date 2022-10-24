/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "universalutils.h"
#include "dfm_event_defines.h"

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
            .arg(QString("media-eject"))
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
            .arg(QString("media-eject"))
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
    QDBusInterface deepinSystemInfo("com.deepin.system.SystemInfo",
                                    "/com/deepin/system/SystemInfo",
                                    "com.deepin.system.SystemInfo",
                                    QDBusConnection::systemBus());
    // 部分数据优先从dbus读取
    // 获取安装的内存总量
    if (deepinSystemInfo.isValid())
        return static_cast<qint64>(deepinSystemInfo.property("MemorySize").toULongLong());

    return -1;
}

void UniversalUtils::computerInformation(QString &cpuinfo, QString &systemType, QString &edition, QString &version)
{
    QDBusInterface systemInfo("com.deepin.daemon.SystemInfo",
                              "/com/deepin/daemon/SystemInfo",
                              "com.deepin.daemon.SystemInfo",
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
        QDBusInterface introspect("com.deepin.SessionManager",
                                  "/com/deepin/StartManager",
                                  "org.freedesktop.DBus.Introspectable",
                                  QDBusConnection::sessionBus());
        introspect.setTimeout(1000);
        QDBusPendingReply<QString> reply = introspect.asyncCallWithArgumentList(QStringLiteral("Introspect"), {});
        reply.waitForFinished();
        if (reply.isFinished() && reply.isValid() && !reply.isError()) {
            QString xmlCode = reply.argumentAt(0).toString();
            if (xmlCode.contains("com.deepin.StartManager")) {
                if (xmlCode.contains("LaunchApp")) {
                    initStatus = true;
                } else {
                    qWarning() << "com.deepin.SessionManager : StartManager doesn't have LaunchApp interface.";
                    initStatus = false;
                }
            } else {
                qWarning() << "com.deepin.SessionManager Introspect error" << xmlCode;
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
    QDBusInterface systemInfo("com.deepin.SessionManager",
                              "/com/deepin/StartManager",
                              "com.deepin.StartManager",
                              QDBusConnection::sessionBus());

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(desktopFile) << QVariant::fromValue(static_cast<uint>(QX11Info::getTimestamp())) << QVariant::fromValue(filePaths);
    systemInfo.asyncCallWithArgumentList(QStringLiteral("LaunchApp"), argumentList);
    return true;
}

bool UniversalUtils::runCommand(const QString &cmd, const QStringList &args, const QString &wd)
{
    if (checkLaunchAppInterface()) {
        qDebug() << "launch cmd by dbus:" << cmd << args;
        QDBusInterface systemInfo("com.deepin.SessionManager",
                                  "/com/deepin/StartManager",
                                  "com.deepin.StartManager",
                                  QDBusConnection::sessionBus());

        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(cmd) << QVariant::fromValue(args);

        if (!wd.isEmpty()) {
            QVariantMap opt = { { "dir", wd } };
            argumentList << QVariant::fromValue(opt);
            systemInfo.asyncCallWithArgumentList(QStringLiteral("RunCommandWithOptions"), argumentList);
        } else {
            systemInfo.asyncCallWithArgumentList(QStringLiteral("RunCommand"), argumentList);
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
    QDBusInterface deepinDockInfo("com.deepin.dde.daemon.Dock",
                                  "/com/deepin/dde/daemon/Dock",
                                  "com.deepin.dde.daemon.Dock",
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

    if ((url1.scheme() == url2.scheme() && path1 == path2))
        return true;
    return false;
}

QString UniversalUtils::getCurrentUser()
{
    QString user;

    QDBusInterface sessionManagerIface("com.deepin.dde.LockService",
                                       "/com/deepin/dde/LockService",
                                       "com.deepin.dde.LockService",
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
            "com.deepin.dde.LockService",
            "/com/deepin/dde/LockService",
            "com.deepin.dde.LockService",
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
