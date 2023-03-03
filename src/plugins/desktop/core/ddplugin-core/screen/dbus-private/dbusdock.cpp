// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
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
