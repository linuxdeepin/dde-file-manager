/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef DBUSTYPE_H
#define DBUSTYPE_H

#include <QtCore>
#include <QtDBus>

#include "durl.h"

class  DiskInfo
{

public:
    QString ID = "";
    QString Name = "";
    QString Type = "";
    QString Path = "";
    QString MountPoint = "";
    QString Icon = "";
    bool CanUnmount = false;
    bool CanEject = false;
    qulonglong Used = 0;
    qulonglong Total = 0;
    qulonglong Free = 0;

    DUrl MountPointUrl;

    /*if true show in computerview for especially used*/
    bool isNativeCustom = false;

    DiskInfo();
    ~DiskInfo();

    friend QDBusArgument &operator<<(QDBusArgument &argument, const DiskInfo &obj);

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, DiskInfo &obj);

    static void registerMetaType();


};


typedef QList<DiskInfo> DiskInfoList;

Q_DECLARE_METATYPE(DiskInfo)
Q_DECLARE_METATYPE(DiskInfoList)


QDebug operator<<(QDebug dbg, const DiskInfo &info);

#endif // DBUSTYPE_H

