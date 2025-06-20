// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSDOCK_H_1465782143
#define DBUSDOCK_H_1465782143

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.deepin.dde.daemon.Dock
 */
struct DockRect{
    qint32 x;
    qint32 y;
    quint32 width;
    quint32 height;

    operator QRect() const
    {
        return QRect(x, y, static_cast<int>(width), static_cast<int>(height));
    }
};
Q_DECLARE_METATYPE(DockRect)
QDBusArgument &operator<<(QDBusArgument &argument, const DockRect &rect);
const QDBusArgument &operator>>(const QDBusArgument &argument, DockRect &rect);
QDebug operator<<(QDebug deg, const DockRect &rect);

class DBusDock: public QDBusAbstractInterface
{
    Q_OBJECT

private slots:
    void __propertyChanged__(const QDBusMessage& msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName !="com.deepin.dde.daemon.Dock")
            return;
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        foreach(const QString &prop, changedProps.keys()) {
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
    { return "com.deepin.dde.daemon.Dock"; }
    static inline const char *staticServiceName()
    { return "com.deepin.dde.daemon.Dock";}
    static inline const char *staticObjectPath()
    { return "/com/deepin/dde/daemon/Dock";}

public:
    explicit DBusDock(QObject *parent = nullptr);

    ~DBusDock();

    Q_PROPERTY(int DisplayMode READ displayMode WRITE setDisplayMode NOTIFY DisplayModeChanged)
    inline int displayMode() const
    { return int(qvariant_cast< int >(property("DisplayMode"))); }
    inline void setDisplayMode(int value)
    { setProperty("DisplayMode", QVariant::fromValue(int(value))); }

    Q_PROPERTY(int HideMode READ hideMode WRITE setHideMode NOTIFY HideModeChanged)
    inline int hideMode() const
    { return qvariant_cast< int >(property("HideMode")); }
    inline void setHideMode(int value)
    { setProperty("HideMode", QVariant::fromValue(value)); }

    Q_PROPERTY(int HideState READ hideState NOTIFY HideStateChanged)
    inline int hideState() const
    { return int(qvariant_cast< int >(property("HideState"))); }

    Q_PROPERTY(int Position READ position WRITE setPosition NOTIFY PositionChanged)
    inline int position() const
    { return int(qvariant_cast< int >(property("Position"))); }
    inline void setPosition(int value)
    { setProperty("Position", QVariant::fromValue(int(value))); }

    Q_PROPERTY(DockRect FrontendWindowRect READ frontendWindowRect NOTIFY FrontendWindowRectChanged)
    inline DockRect frontendWindowRect() const
    { return qvariant_cast< DockRect >(property("FrontendWindowRect")); }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> ActivateWindow(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("ActivateWindow"), argumentList);
    }

    inline QDBusPendingReply<> CloseWindow(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("CloseWindow"), argumentList);
    }

    inline QDBusPendingReply<QStringList> GetEntryIDs()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetEntryIDs"), argumentList);
    }

    inline QDBusPendingReply<> MoveEntry(const int oldIndex, const int newIndex)
    {
        QList<QVariant> args;
        args << oldIndex << newIndex;

        return asyncCallWithArgumentList(QStringLiteral("MoveEntry"), args);
    }

    inline QDBusPendingReply<bool> RequestDock(const QString &appDesktop, const int index = -1)
    {
        QList<QVariant> args;
        args << appDesktop << index;

        return asyncCallWithArgumentList(QStringLiteral("RequestDock"), args);
    }

    inline QDBusPendingReply<bool> IsDocked(const QString &appDesktop)
    {
        QList<QVariant> args;
        args << appDesktop;

        return asyncCallWithArgumentList(QStringLiteral("IsDocked"), args);
    }

    inline QDBusPendingReply<bool> IsOnDock(const QString &appDesktop)
    {
        QList<QVariant> args;
        args << appDesktop;

        return asyncCallWithArgumentList(QStringLiteral("IsOnDock"), args);
    }

    inline QDBusPendingReply<bool> RequestUndock(const QString &appDesktop)
    {
        QList<QVariant> args;
        args << appDesktop;

        return asyncCallWithArgumentList(QStringLiteral("RequestUndock"), args);
    }

    inline QDBusPendingReply<> SetFrontendWindowRect(const int x, const int y, const quint32 width, const quint32 height)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(x) << QVariant::fromValue(y) << QVariant::fromValue(width) << QVariant::fromValue(height);
        return asyncCallWithArgumentList(QStringLiteral("SetFrontendWindowRect"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    // begin property changed signals
    void DisplayModeChanged();
    void HideModeChanged();
    void HideStateChanged();
    void PositionChanged();
    void FrontendWindowRectChanged();
};

#endif
