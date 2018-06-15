/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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

#include "udisks2_dbus_common.h"
#include "objectmanager_interface.h"

#include <QDBusArgument>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QXmlStreamReader>

DFM_BEGIN_NAMESPACE

namespace UDisks2 {
Q_GLOBAL_STATIC_WITH_ARGS(OrgFreedesktopDBusObjectManagerInterface, omGlobal, (UDISKS2_SERVICE, "/org/freedesktop/UDisks2", QDBusConnection::systemBus()))

bool interfaceIsExistes(const QString &path, const QString &interface)
{
    QDBusInterface ud2(UDISKS2_SERVICE, path, "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus());
    QDBusReply<QString> reply = ud2.call("Introspect");
    QXmlStreamReader xml_parser(reply.value());

    while (!xml_parser.atEnd()) {
        xml_parser.readNext();

        if (xml_parser.tokenType() == QXmlStreamReader::StartElement
                && xml_parser.name().toString() == "interface") {
            const QString &name = xml_parser.attributes().value("name").toString();

            if (name == interface) {
                return true;
            }
        }
    }

    return false;
}

OrgFreedesktopDBusObjectManagerInterface *objectManager()
{
    if (!omGlobal.exists()) {
        qDBusRegisterMetaType<QMap<QString, QVariantMap>>();
        qDBusRegisterMetaType<QList<QPair<QString, QVariantMap>>>();
        qDBusRegisterMetaType<QByteArrayList>();
        qDBusRegisterMetaType<QPair<QString,QVariantMap>>();
        qDBusRegisterMetaType<QMap<QDBusObjectPath,QMap<QString,QVariantMap>>>();

        QMetaType::registerDebugStreamOperator<QList<QPair<QString, QVariantMap>>>();
    }

    return omGlobal;
}
}

DFM_END_NAMESPACE

QDBusArgument &operator<<(QDBusArgument &argument, const dde_file_manager::UDisks2::SmartAttribute &mystruct)
{
    argument.beginStructure();
    argument << mystruct.id
             << mystruct.name
             << mystruct.flags
             << mystruct.value
             << mystruct.worst
             << mystruct.threshold
             << mystruct.pretty
             << mystruct.pretty_unit
             << mystruct.expansion;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, dde_file_manager::UDisks2::SmartAttribute &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.id
            >> mystruct.name
            >> mystruct.flags
            >> mystruct.value
            >> mystruct.worst
            >> mystruct.threshold
            >> mystruct.pretty
            >> mystruct.pretty_unit
            >> mystruct.expansion;
    argument.endStructure();

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const dde_file_manager::UDisks2::ActiveDeviceInfo &mystruct)
{
    argument.beginStructure();
    argument << mystruct.block
             << mystruct.slot
             << mystruct.state
             << mystruct.num_read_errors
             << mystruct.expansion;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, dde_file_manager::UDisks2::ActiveDeviceInfo &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.block
            >> mystruct.slot
            >> mystruct.state
            >> mystruct.num_read_errors
            >> mystruct.expansion;
    argument.endStructure();

    return argument;
}
