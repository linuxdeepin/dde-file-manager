// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
