// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusdock.h"

/*
 * Implementation of interface class DBusDock
 */
DBusDock::DBusDock(QObject *parent)
    : QDBusAbstractInterface(staticServiceName(), staticObjectPath(), staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
    qDBusRegisterMetaType<DockRect>();
    QDBusConnection::sessionBus().connect(this->service(), this->path(),
            "org.freedesktop.DBus.Properties",  "PropertiesChanged",
            this, SLOT(__propertyChanged__(QDBusMessage)));
}

DBusDock::~DBusDock()
{
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties"
                     ,  "PropertiesChanged", this, SLOT(__propertyChanged__(QDBusMessage)));
}

QDBusArgument &operator<<(QDBusArgument &argument, const DockRect &rect)
{
    argument.beginStructure();
    argument << rect.x << rect.y << rect.width << rect.height;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DockRect &rect)
{
    argument.beginStructure();
    argument >> rect.x >> rect.y >> rect.width >> rect.height;
    argument.endStructure();
    return argument;
}

QDebug operator<<(QDebug deg, const DockRect &rect)
{
    qDebug() << "x:" << rect.x << "y:" << rect.y << "width:" << rect.width << "height:" << rect.height;

    return deg;
}

DockInfo *DockInfo::ins()
{
    static DockInfo ins;
    return  &ins;
}

DBusDock *DockInfo::dock() const
{
    return m_dock;
}

DockInfo::DockInfo(QObject *parent)
    : QObject (parent)
{
    qInfo() << "create dbus dock.";
    m_dock = new DBusDock(this);
    m_dock->setTimeout(500);
    qInfo() << "create dbus dock end";
}
