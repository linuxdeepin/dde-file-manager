#ifndef UDISKDEVICEINFO_H
#define UDISKDEVICEINFO_H

#include "../filemanager/models/abstractfileinfo.h"

#include <QDBusInterface>
#include <QString>
#include <QDebug>
#include <QDBusArgument>
#include "dbusinterface/dbustype.h"

class UDiskDeviceInfo : public QObject ,public AbstractFileInfo
{
    Q_OBJECT
public:
    UDiskDeviceInfo();
    UDiskDeviceInfo(UDiskDeviceInfo * info);
    UDiskDeviceInfo(const DUrl &url);
    UDiskDeviceInfo(const QString &url);
    UDiskDeviceInfo(const QDBusObjectPath &path);
    UDiskDeviceInfo(const DiskInfo &diskInfo);
    ~UDiskDeviceInfo();
    void setDiskInfo(const DiskInfo &diskInfo);
    DiskInfo getDiskInfo();
    QString getId();
    QString getName();
    QString getType();
    QString getPath();
    QString getMountPoint() const;
    QString getIcon();
    bool canEject();
    bool canUnmount();
    qulonglong getUsed();
    qulonglong getTotal();
    qint64 size();
    QString displayName() const;

private:
    DiskInfo m_diskInfo;

public:
    bool isCanRename() const Q_DECL_OVERRIDE;
    QIcon fileIcon() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;
    DUrl parentUrl() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;
};

#endif // UDISKDEVICEINFO_H
