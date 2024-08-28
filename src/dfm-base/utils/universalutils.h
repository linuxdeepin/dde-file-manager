// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNIVERSALUTILS_H
#define UNIVERSALUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <QString>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusUnixFileDescriptor>

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

    static bool checkLaunchAppInterface();
    static bool launchAppByDBus(const QString &desktopFile, const QStringList &filePaths);
    static bool runCommand(const QString &cmd, const QStringList &args, const QString &wd = QString());
    static int dockHeight();
    static QMap<QString, QString> getKernelParameters();
    static bool isInLiveSys();

    static QVariantHash convertFromQMap(const QVariantMap map);

    static bool urlEquals(const QUrl &url1, const QUrl &url2);
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
};

}

#endif   // UNIVERSALUTILS_H
