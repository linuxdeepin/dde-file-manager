#include "dbusinforet.h"

DBusInfoRet::DBusInfoRet()
{

}

DBusInfoRet::~DBusInfoRet()
{

}

QDBusArgument &operator<<(QDBusArgument &argument, const DBusInfoRet &obj)
{
    argument.beginStructure();
    argument << obj.ServicePath << obj.InterfacePath;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusInfoRet &obj)
{
    argument.beginStructure();
    argument >> obj.ServicePath >> obj.InterfacePath;
    argument.endStructure();
    return argument;
}

QDebug operator<<(QDebug dbg, const DBusInfoRet &obj)
{
    dbg.nospace() << "{ ";
    dbg.nospace() << "ServicePath :" << obj.ServicePath << ", ";
    dbg.nospace() << "InterfacePath :" << obj.InterfacePath.path();
    dbg.nospace() << " }";
    return dbg;
}

void DBusInfoRet::registerMetaType()
{
    qRegisterMetaType<DBusInfoRet>(QT_STRINGIFY(DBusInfoRet));
    qDBusRegisterMetaType<DBusInfoRet>();
}

