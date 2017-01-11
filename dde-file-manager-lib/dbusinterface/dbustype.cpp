
#include "dbustype.h"

DiskInfo::DiskInfo(){

}

DiskInfo::~DiskInfo(){

}

void DiskInfo::registerMetaType()
{
    qRegisterMetaType<DiskInfo>(QT_STRINGIFY(DiskInfo));
    qDBusRegisterMetaType<DiskInfo>();
    qRegisterMetaType<DiskInfoList>(QT_STRINGIFY(DiskInfoList));
    qDBusRegisterMetaType<DiskInfoList>();
}


QDBusArgument &operator<<(QDBusArgument &argument, const DiskInfo &obj)
{
    argument.beginStructure();
    argument << obj.ID << obj.Name;
    argument << obj.Type << obj.Path;
    argument << obj.MountPoint << obj.Icon;
    argument << obj.CanUnmount << obj.CanEject;
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
    argument >> obj.CanUnmount >> obj.CanEject;
    argument >> obj.Used >> obj.Total;

    obj.Total = obj.Total * 1024;
    obj.Used = obj.Used * 1024;
    obj.Free = obj.Total - obj.Used;

    argument.endStructure();

    obj.MountPointUrl.setUrl(obj.MountPoint);
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
    dbg.nospace() << "CanUnmount: " << info.CanUnmount << ",";
    dbg.nospace() << "CanEject: " << info.CanEject << ",";
    dbg.nospace() << "Used: " << info.Used<< ",";
    dbg.nospace() << "Total: " << info.Total<< ",";
    dbg.nospace() << "MountPointUrl: " << info.MountPointUrl << ",";
    dbg.nospace() << ")";
    return dbg;
}
