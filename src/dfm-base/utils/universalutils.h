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
#ifndef UNIVERSALUTILS_H
#define UNIVERSALUTILS_H

#include "dfm-base/dfm_base_global.h"

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

    static QString getCurrentUser();

    static void userChange(QObject *obj, const char *cslot = nullptr);

    static void prepareForSleep(QObject *obj, const char *cslot = nullptr);
};

}

#endif   // UNIVERSALUTILS_H
