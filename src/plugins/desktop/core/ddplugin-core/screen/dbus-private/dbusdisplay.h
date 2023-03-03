// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSDISPLAY_H
#define DBUSDISPLAY_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

struct DisplayRect {
    qint16 x;
    qint16 y;
    quint16 width;
    quint16 height;

    operator QRect() const
    {
        return QRect(x, y, width, height);
    }
};

Q_DECLARE_METATYPE(DisplayRect)

QDBusArgument &operator<<(QDBusArgument &argument, const DisplayRect &rect);
const QDBusArgument &operator>>(const QDBusArgument &argument, DisplayRect &rect);
QDebug operator<<(QDebug deg, const DisplayRect &rect);

/*
 * Proxy class for interface com.deepin.daemon.Display
 */
class DBusDisplay: public QDBusAbstractInterface
{
    Q_OBJECT

    Q_SLOT void __propertyChanged__(const QDBusMessage &msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName != "com.deepin.daemon.Display")
            return;
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        QStringList keys = changedProps.keys();
        foreach (const QString &prop, keys) {
            const QMetaObject *self = metaObject();
            for (int i = self->propertyOffset(); i < self->propertyCount(); ++i) {
                QMetaProperty p = self->property(i);
                if (p.name() == prop) {
                    Q_EMIT p.notifySignal().invoke(this);
                }
            }
        }
    }
public:
    static inline const char *staticInterfaceName()
    {
        return "com.deepin.daemon.Display";
    }
    static inline const char *staticServiceName()
    {
        return "com.deepin.daemon.Display";
    }
    static inline const char *staticObjectPath()
    {
        return "/com/deepin/daemon/Display";
    }

public:
    explicit DBusDisplay(QObject *parent = nullptr);

    ~DBusDisplay();

    Q_PROPERTY(uchar DisplayMode READ displayMode NOTIFY DisplayModeChanged)
    inline short displayMode() const
    {
        return qvariant_cast< uchar >(property("DisplayMode"));
    }

    Q_PROPERTY(bool HasChanged READ hasChanged NOTIFY HasChangedChanged)
    inline bool hasChanged() const
    {
        return qvariant_cast< bool >(property("HasChanged"));
    }

    Q_PROPERTY(QList<QDBusObjectPath> Monitors READ monitors NOTIFY MonitorsChanged)
    inline QList<QDBusObjectPath> monitors() const
    {
        return qvariant_cast< QList<QDBusObjectPath> >(property("Monitors"));
    }

    Q_PROPERTY(QString Primary READ primary NOTIFY PrimaryChanged)
    inline QString primary() const
    {
        return qvariant_cast< QString >(property("Primary"));
    }

    Q_PROPERTY(DisplayRect PrimaryRect READ primaryRect NOTIFY PrimaryRectChanged)
    inline DisplayRect primaryRect() const
    {
        return qvariant_cast< DisplayRect >(property("PrimaryRect"));
    }

    Q_PROPERTY(ushort ScreenHeight READ screenHeight NOTIFY ScreenHeightChanged)
    inline ushort screenHeight() const
    {
        return qvariant_cast< ushort >(property("ScreenHeight"));
    }

    Q_PROPERTY(ushort ScreenWidth READ screenWidth NOTIFY ScreenWidthChanged)
    inline ushort screenWidth() const
    {
        return qvariant_cast< ushort >(property("ScreenWidth"));
    }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> Apply()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("Apply"), argumentList);
    }

    inline QDBusPendingReply<> AssociateTouchScreen(const QString &in0, const QString &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("AssociateTouchScreen"), argumentList);
    }

    inline QDBusPendingReply<> ChangeBrightness(const QString &in0, double in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("ChangeBrightness"), argumentList);
    }

    inline QDBusPendingReply<QStringList> ListOutputNames()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("ListOutputNames"), argumentList);
    }

    inline QDBusPendingReply<> JoinMonitor(const QString &in0, const QString &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("JoinMonitor"), argumentList);
    }

    inline QDBusPendingReply<QString> QueryCurrentPlanName()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("QueryCurrentPlanName"), argumentList);
    }

    inline QDBusPendingReply<int> QueryOutputFeature(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("QueryOutputFeature"), argumentList);
    }

    inline QDBusPendingReply<> Reset()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("Reset"), argumentList);
    }

    inline QDBusPendingReply<> ResetBrightness(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("ResetBrightness"), argumentList);
    }

    inline QDBusPendingReply<> ResetChanges()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("ResetChanges"), argumentList);
    }

    inline QDBusPendingReply<> SaveChanges()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("SaveChanges"), argumentList);
    }

    inline QDBusPendingReply<> SetBrightness(const QString &in0, double in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("SetBrightness"), argumentList);
    }

    inline QDBusPendingReply<> SetPrimary(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SetPrimary"), argumentList);
    }

    inline QDBusPendingReply<> SplitMonitor(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SplitMonitor"), argumentList);
    }

    inline QDBusPendingReply<> SwitchMode(short in0, const QString &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("SwitchMode"), argumentList);
    }

    inline QDBusPendingReply<uchar> GetRealDisplayMode()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetRealDisplayMode"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void DisplayModeChanged();
    void HasChangedChanged();
    void MonitorsChanged();
    void PrimaryChanged();
    void PrimaryRectChanged();
    void ScreenHeightChanged();
    void ScreenWidthChanged();
};

#endif
