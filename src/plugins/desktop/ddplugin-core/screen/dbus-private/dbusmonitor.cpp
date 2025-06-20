// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusmonitor.h"

DBusMonitor::DBusMonitor(const QString &objectPath,QObject *parent)
    : QDBusAbstractInterface(staticServiceName(), objectPath, staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
    QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(QDBusMessage)));
}

DBusMonitor::~DBusMonitor()
{
    QDBusConnection::sessionBus().disconnect(
                service(), path(), "org.freedesktop.DBus.Properties",
                "PropertiesChanged",  "sa{sv}as", this, SLOT(propertyChanged(QDBusMessage)));
}

QRect DBusMonitor::rect() const
{
    return QRect(x(),y(),width(),height());
}
