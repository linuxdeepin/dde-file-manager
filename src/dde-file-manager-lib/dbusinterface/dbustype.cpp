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

#include "dbustype.h"

DiskInfo::DiskInfo(){

}

DiskInfo::~DiskInfo(){

}

void DiskInfo::registerMetaType()
{
    qRegisterMetaType<DiskInfo>(QT_STRINGIFY(DiskInfo));
    qDBusRegisterMetaType<DiskInfo>();
    qRegisterMetaType<DiskInfoList>(QT_STRINGIFY(DiskInfoList));
    qDBusRegisterMetaType<DiskInfoList>();
}


QDBusArgument &operator<<(QDBusArgument &argument, const DiskInfo &obj)
{
    argument.beginStructure();
    argument << obj.ID << obj.Name;
    argument << obj.Type << obj.Path;
    argument << obj.MountPoint << obj.Icon;
    argument << obj.CanUnmount << obj.CanEject;
    argument << obj.Used << obj.Total;
    argument.endStructure();
    return argument;
}


const QDBusArgument &operator>>(const QDBusArgument &argument, DiskInfo &obj)
{
    argument.beginStructure();
    argument >> obj.ID >> obj.Name;
    argument >> obj.Type >> obj.Path;
    argument >> obj.MountPoint >>  obj.Icon;
    argument >> obj.CanUnmount >> obj.CanEject;
    argument >> obj.Used >> obj.Total;

    obj.Total = obj.Total * 1024;
    obj.Used = obj.Used * 1024;
    obj.Free = obj.Total - obj.Used;

    argument.endStructure();

    obj.MountPointUrl.setUrl(obj.MountPoint);
    return argument;
}


QDebug operator<<(QDebug dbg, const DiskInfo &info)
{
    dbg.nospace() << "DiskInfo(";
    dbg.nospace() << "ID: " << info.ID << ",";
    dbg.nospace() << "Name: " << info.Name << ",";
    dbg.nospace() << "Type: " << info.Type << ",";
    dbg.nospace() << "Path: " << info.Path << ",";
    dbg.nospace() << "MountPoint: " << info.MountPoint << ",";
    dbg.nospace() << "Icon: " << info.Icon << ",";
    dbg.nospace() << "CanUnmount: " << info.CanUnmount << ",";
    dbg.nospace() << "CanEject: " << info.CanEject << ",";
    dbg.nospace() << "Used: " << info.Used<< ",";
    dbg.nospace() << "Total: " << info.Total<< ",";
    dbg.nospace() << "MountPointUrl: " << info.MountPointUrl << ",";
    dbg.nospace() << ")";
    return dbg;
}
