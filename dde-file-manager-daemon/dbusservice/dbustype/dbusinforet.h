#ifndef DBUSINFORET_H
#define DBUSINFORET_H

#include <QtCore>
#include <QtDBus>


class DBusInfoRet
{
public:

    QString ServicePath;
    QDBusObjectPath InterfacePath;

    DBusInfoRet();
    ~DBusInfoRet();

    friend QDBusArgument &operator<<(QDBusArgument &argument, const DBusInfoRet &obj);

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, DBusInfoRet &obj);

    friend QDebug operator<<(QDebug dbg, const DBusInfoRet &obj);

    static void registerMetaType();
};

Q_DECLARE_METATYPE(DBusInfoRet)

#endif // DBUSINFORET_H
