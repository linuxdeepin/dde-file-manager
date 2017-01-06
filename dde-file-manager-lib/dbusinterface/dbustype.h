#ifndef DBUSTYPE_H
#define DBUSTYPE_H

#include <QtCore>
#include <QtDBus>

#include "durl.h"

class  DiskInfo
{

public:
    QString ID = "";
    QString Name = "";
    QString Type = "";
    QString Path = "";
    QString MountPoint = "";
    QString Icon = "";
    bool CanUnmount = false;
    bool CanEject = false;
    qulonglong Used = 0;
    qulonglong Total = 0;
    qulonglong Free = 0;

    DUrl MountPointUrl;

    /*if true show in computerview for especially used*/
    bool isNativeCustom = false;

    DiskInfo();
    ~DiskInfo();

    friend QDBusArgument &operator<<(QDBusArgument &argument, const DiskInfo &obj);

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, DiskInfo &obj);

    static void registerMetaType();


};


typedef QList<DiskInfo> DiskInfoList;

Q_DECLARE_METATYPE(DiskInfo)
Q_DECLARE_METATYPE(DiskInfoList)


QDebug operator<<(QDebug dbg, const DiskInfo &info);

#endif // DBUSTYPE_H

