/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             xinglinkun<xinglinkun@uniontech.com>
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

#ifndef DEEPIN_WM_H_1458115332
#define DEEPIN_WM_H_1458115332

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.deepin.wm
 */
class DeepinWM: public QDBusAbstractInterface
{
    Q_OBJECT

    Q_SLOT void __propertyChanged__(const QDBusMessage& msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName !="com.deepin.wm")
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
    static inline const char *staticInterfaceName()
    { return "com.deepin.wm"; }

public:
    DeepinWM(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~DeepinWM();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> CancelHideWindows()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("CancelHideWindows"), argumentList);
    }

    inline QDBusPendingReply<> PerformAction(int type)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(type);
        return asyncCallWithArgumentList(QStringLiteral("PerformAction"), argumentList);
    }

    inline QDBusPendingReply<> RequestHideWindows()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("RequestHideWindows"), argumentList);
    }

    inline QDBusPendingReply<> ToggleDebug()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("ToggleDebug"), argumentList);
    }

    inline QDBusPendingReply<> SetTransientBackground(const QString &uri)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(uri);
        return asyncCallWithArgumentList(QStringLiteral("SetTransientBackground"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void BackgroundChanged();
// begin property changed signals
};

#endif
