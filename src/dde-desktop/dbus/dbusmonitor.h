/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
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

#ifndef DBUSMONITOR_H
#define DBUSMONITOR_H

#include <QObject>
#include <QtDBus/QtDBus>

class DBusMonitor : public QDBusAbstractInterface
{
    Q_OBJECT
    Q_SLOT void __propertyChanged__(const QDBusMessage& msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName !="com.deepin.daemon.Display.Monitor")
            return;
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        QStringList keys = changedProps.keys();
        foreach(const QString &prop, keys) {
        const QMetaObject* self = metaObject();
            for (int i=self->propertyOffset(); i < self->propertyCount(); ++i) {
                QMetaProperty p = self->property(i);
                if (p.name() == prop) {
                Q_EMIT p.notifySignal().invoke(this);
                }
            }
        }
   }
public:
    static inline const char *staticServiceName()
    { return "com.deepin.daemon.Display"; }
    static inline const char *staticInterfaceName()
    { return "com.deepin.daemon.Display.Monitor"; }
public:
    DBusMonitor(const QString &objectPath,QObject *parent = nullptr);
    ~DBusMonitor();

    Q_PROPERTY(QString Name READ name)
    inline QString name() const
    { return qvariant_cast< QString >(property("Name")); }

    Q_PROPERTY(qint16 X READ x NOTIFY monitorRectChanged)
    inline qint16 x() const
    { return qvariant_cast< qint16 >(property("X")); }

    Q_PROPERTY(qint16 Y READ y NOTIFY monitorRectChanged)
    inline qint16 y() const
    { return qvariant_cast< qint16 >(property("Y") ); }

    Q_PROPERTY(quint16 Width READ width NOTIFY monitorRectChanged)
    inline quint16 width() const
    { return qvariant_cast< quint16 >(property("Width")); }

    Q_PROPERTY(quint16 Height READ height NOTIFY monitorRectChanged)
    inline quint16 height() const
    { return qvariant_cast< quint16 >(property("Height")); }

    Q_PROPERTY(bool Enabled READ enabled NOTIFY monitorEnabledChanged)
    inline bool enabled() const
    { return qvariant_cast< bool >(property("Enabled")); }
signals:
    void monitorRectChanged();
    void monitorEnabledChanged();
public:
    QRect rect() const;
};

#endif // DBUSMONITOR_H
