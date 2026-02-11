// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbushelper.h"

DDPCORE_USE_NAMESPACE

DBusHelper::DBusHelper(QObject *parent)
    : QObject{parent}
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    fmInfo() << "create dock and display dbus interface.";
    m_dock = new DBusDock(this);
    m_display = new DBusDisplay(this);
    fmInfo() << "the dbus interface: dock is" << isDockEnable()  << "display is" << isDisplayEnable();
}

DBusHelper *DBusHelper::ins()
{
    static DBusHelper ins;
    return  &ins;
}

bool DBusHelper::isDockEnable()
{
    if (auto ifs = QDBusConnection::sessionBus().interface())
        return ifs->isServiceRegistered(DBusDock::staticServiceName());

    return false;
}

bool DBusHelper::isDisplayEnable()
{
    if (auto ifs = QDBusConnection::sessionBus().interface())
        return ifs->isServiceRegistered(DBusDisplay::staticServiceName());

    return false;
}

DBusDock *DBusHelper::dock() const
{
    return m_dock;
}

DBusDisplay *DBusHelper::display() const
{
    return m_display;
}

#if 0
DBusMonitor *DBusHelper::createMonitor(const QString &path)
{
    return new DBusMonitor(path);
}
#endif

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
    fmDebug() << "x:" << rect.x << "y:" << rect.y << "width:" << rect.width << "height:" << rect.height;

    return deg;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DisplayRect &rect)
{
    argument.beginStructure();
    argument << rect.x << rect.y << rect.width << rect.height;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DisplayRect &rect)
{
    argument.beginStructure();
    argument >> rect.x >> rect.y >> rect.width >> rect.height;
    argument.endStructure();
    return argument;
}

QDebug operator<<(QDebug deg, const DisplayRect &rect)
{
    fmDebug() << "x:" << rect.x << "y:" << rect.y << "width:" << rect.width << "height:" << rect.height;

    return deg;
}
