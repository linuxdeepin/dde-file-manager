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

#include "dbusinforet.h"

DBusInfoRet::DBusInfoRet()
{

}

DBusInfoRet::~DBusInfoRet()
{

}

QDBusArgument &operator<<(QDBusArgument &argument, const DBusInfoRet &obj)
{
    argument.beginStructure();
    argument << obj.ServicePath << obj.InterfacePath;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusInfoRet &obj)
{
    argument.beginStructure();
    argument >> obj.ServicePath >> obj.InterfacePath;
    argument.endStructure();
    return argument;
}

QDebug operator<<(QDebug dbg, const DBusInfoRet &obj)
{
    dbg.nospace() << "{ ";
    dbg.nospace() << "ServicePath :" << obj.ServicePath << ", ";
    dbg.nospace() << "InterfacePath :" << obj.InterfacePath.path();
    dbg.nospace() << " }";
    return dbg;
}

void DBusInfoRet::registerMetaType()
{
    qRegisterMetaType<DBusInfoRet>(QT_STRINGIFY(DBusInfoRet));
    qDBusRegisterMetaType<DBusInfoRet>();
}

