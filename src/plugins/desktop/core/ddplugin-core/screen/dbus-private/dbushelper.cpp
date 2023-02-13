// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbushelper.h"

DDPCORE_USE_NAMESPACE

DBusHelper::DBusHelper(QObject *parent)
    : QObject{parent}
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    m_dock = new DBusDock(this);
    m_display = new DBusDisplay(this);
}

DBusHelper *DBusHelper::ins()
{
    static DBusHelper ins;
    return  &ins;
}

DBusDock *DBusHelper::dock() const
{
    return m_dock;
}

DBusDisplay *DBusHelper::display() const
{
    return m_display;
}

DBusMonitor *DBusHelper::createMonitor(const QString &path)
{
    return new DBusMonitor(path);
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
    qDebug() << "x:" << rect.x << "y:" << rect.y << "width:" << rect.width << "height:" << rect.height;

    return deg;
}
