// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNIVERSALUTILS_H
#define UNIVERSALUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <QString>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusUnixFileDescriptor>
#include <QFontMetrics>
#include <QModelIndex>

namespace dfmbase {

class UniversalUtils
{
public:
    static bool inMainThread();
    static void notifyMessage(const QString &msg);
    static void notifyMessage(const QString &title, const QString &msg);
    static QString userLoginState();
    static quint32 currentLoginUser();
    static bool isLogined();
    static void blockShutdown(QDBusReply<QDBusUnixFileDescriptor> &replay);
    static qint64 computerMemory();
    static void computerInformation(QString &cpuinfo, QString &systemType, QString &Edition, QString &version);
    static double sizeFormat(qint64 size, QString &unit);
    static QString sizeFormat(qint64 size, int percision);

    static bool runCommand(const QString &cmd, const QStringList &args, const QString &wd = QString());
    static int dockHeight();
    static QMap<QString, QString> getKernelParameters();
    static bool isInLiveSys();

    static QVariantHash convertFromQMap(const QVariantMap map);

    static bool urlEquals(const QUrl &url1, const QUrl &url2);
    static bool urlEqualsWithQuery(const QUrl &url1, const QUrl &url2);
    static bool urlsTransformToLocal(const QList<QUrl> &sourceUrls, QList<QUrl> *targetUrls);
    static bool urlTransformToLocal(const QUrl &sourceUrl, QUrl *targetUrls);

    static QString getCurrentUser();

    static void userChange(QObject *obj, const char *cslot = nullptr);

    static void prepareForSleep(QObject *obj, const char *cslot = nullptr);
    static bool isNetworkRoot(const QUrl &url);

    static bool isParentUrl(const QUrl &child, const QUrl &parent);
    static bool isParentOnly(const QUrl &child, const QUrl &parent);

    static QString covertUrlToLocalPath(const QString &url);
    static void boardCastPastData(const QUrl &sourcPath, const QUrl &targetPath,
                                  const QList<QUrl> &files);

    static int getTextLineHeight(const QModelIndex &index, const QFontMetrics &fontMetrics);
    static int getTextLineHeight(const QString &text, const QFontMetrics &fontMetrics);

    /*!
     * \brief 锁定屏幕保护程序，防止系统进入休眠或屏保模式。
     * \return 成功时返回有效的 cookie，失败时返回 0。
     */
    static uint32_t lockScreenSaver();

    /*!
     * \brief 解除屏幕保护程序的锁定。
     * \param cookie 由 lockScreenSaver 返回的 cookie。
     * \return 成功时返回 true，失败时返回 false。
     */
    static bool unlockScreenSaver(const uint32_t cookie);

    /*!
     * \brief 检查指定的 DBus 服务是否存在。
     * \param service DBus 服务名称。
     * \param isSystemDbus 是否为系统总线。
     * \return 服务存在时返回 true，否则返回 false。
     */
    static bool checkDbusService(const QString &service, bool isSystemDbus);
};

}

#endif   // UNIVERSALUTILS_H
