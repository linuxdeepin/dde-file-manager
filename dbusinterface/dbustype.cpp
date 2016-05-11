
#include "dbustype.h"

DiskInfo::DiskInfo(){

}

DiskInfo::~DiskInfo(){

}

void DiskInfo::registerMetaType()
{
    qRegisterMetaType<DiskInfo>("DiskInfo");
    qDBusRegisterMetaType<DiskInfo>();
    qRegisterMetaType<DiskInfoList>("DiskInfoList");
    qDBusRegisterMetaType<DiskInfoList>();
}


QDBusArgument &operator<<(QDBusArgument &argument, const DiskInfo &obj)
{
    argument.beginStructure();
    argument << obj.ID << obj.Name;
    argument << obj.Type << obj.Path;
    argument << obj.MountPoint << obj.Icon;
    argument << obj.CanEject << obj.CanUnmount;
    argument << obj.Used << obj.Total;
    argument.endStructure();
    return argument;
}


const QDBusArgument &operator>>(const QDBusArgument &argument, DiskInfo &obj)
{
    argument.beginStructure();
    argument >> obj.ID >> obj.Name;
    argument >> obj.Type >> obj.Path;
    argument >> obj.MountPoint >>  obj.Icon;
    argument >> obj.CanEject >> obj.CanUnmount;
    argument >> obj.Used >> obj.Total;
    argument.endStructure();
    return argument;
}


QDebug operator<<(QDebug dbg, const DiskInfo &info)
{
    dbg.nospace() << "DiskInfo(";
    dbg.nospace() << "ID: " << info.ID << ",";
    dbg.nospace() << "Name: " << info.Name << ",";
    dbg.nospace() << "Type: " << info.Type << ",";
    dbg.nospace() << "Path: " << info.Path << ",";
    dbg.nospace() << "MountPoint: " << info.MountPoint << ",";
    dbg.nospace() << "Icon: " << info.Icon << ",";
    dbg.nospace() << "CanEject: " << info.CanEject << ",";
    dbg.nospace() << "CanUnmount: " << info.CanUnmount << ",";
    dbg.nospace() << "Used: " << info.Used<< ",";
    dbg.nospace() << "Total: " << info.Total<< ",";
    dbg.nospace() << ")";
    return dbg;
}
