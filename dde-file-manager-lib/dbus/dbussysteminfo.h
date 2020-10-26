#ifndef DBUSSYSTEMINFO_H
#define DBUSSYSTEMINFO_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

class DBusSystemInfo : public QDBusAbstractInterface
{
    Q_OBJECT

    Q_SLOT void __propertyChanged__(const QDBusMessage& msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName !="com.deepin.daemon.SystemInfo")
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
    { return "com.deepin.daemon.SystemInfo"; }
    static inline const char *staticServiceName()
    { return "com.deepin.daemon.SystemInfo"; }
    static inline const char *staticObjectPath()
    { return "/com/deepin/daemon/SystemInfo"; }

public:
    explicit DBusSystemInfo(QObject *parent = nullptr);
    ~DBusSystemInfo();

    Q_PROPERTY(qint64 SystemType READ systemType)
    inline qint64 systemType() const
    { return qvariant_cast< qint64 >(property("SystemType")); }

    Q_PROPERTY(QString DistroDesc READ distroDesc NOTIFY DistroDescChanged)
    inline QString distroDesc() const
    { return qvariant_cast< QString >(property("DistroDesc")); }

    Q_PROPERTY(QString DistroID READ distroID NOTIFY DistroIDChanged)
    inline QString distroID() const
    { return qvariant_cast< QString >(property("DistroID")); }

    Q_PROPERTY(QString DistroVer READ distroVer NOTIFY DistroVerChanged)
    inline QString distroVer() const
    { return qvariant_cast< QString >(property("DistroVer")); }

    Q_PROPERTY(QString Processor READ processor)
    inline QString processor() const
    { return qvariant_cast< QString >(property("Processor")); }

    Q_PROPERTY(QString Version READ version NOTIFY VersionChanged)
    inline QString version() const
    { return qvariant_cast< QString >(property("Version")); }

    Q_PROPERTY(quint64 DiskCap READ diskCap NOTIFY DiskCapChanged)
    inline quint64 diskCap() const
    { return qvariant_cast< quint64 >(property("DiskCap")); }

    Q_PROPERTY(quint64 MemoryCap READ memoryCap NOTIFY MemoryCapChanged)
    inline quint64 memoryCap() const
    { return qvariant_cast< quint64 >(property("MemoryCap")); }

Q_SIGNALS:
    void DistroDescChanged();
    void DistroIDChanged();
    void DistroVerChanged();
    void VersionChanged();
    void DiskCapChanged();
    void MemoryCapChanged();
};

#endif // DBUSSYSTEMINFO_H
