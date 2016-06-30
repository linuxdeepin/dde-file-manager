#ifndef UDISKDEVICEINFO_H
#define UDISKDEVICEINFO_H

#include "../filemanager/models/abstractfileinfo.h"

#include <QDBusInterface>
#include <QString>
#include <QDebug>
#include <QDBusArgument>
#include "dbusinterface/dbustype.h"
#include "../models/menuactiontype.h"

class UDiskDeviceInfo : public QObject ,public AbstractFileInfo
{
    Q_OBJECT
public:
    enum MediaType
    {
        unknown,
        native,
        phone,
        iphone,
        removable,
        camera,
        network,
    };

    UDiskDeviceInfo();
    UDiskDeviceInfo(UDiskDeviceInfo * info);
    UDiskDeviceInfo(const DUrl &url);
    UDiskDeviceInfo(const QString &url);
    UDiskDeviceInfo(const DiskInfo &diskInfo);
    ~UDiskDeviceInfo();
    void setDiskInfo(const DiskInfo &diskInfo);
    DiskInfo getDiskInfo();
    QString getId();
    QString getName();
    QString getType() const;
    QString getPath();
    QString getMountPoint();
    QString getIcon();
    bool canEject();
    bool canUnmount();
    qulonglong getUsed();
    qulonglong getTotal();
    qint64 size();
    QString displayName() const;
    MediaType getMediaType() const;
    QString deviceTypeDisplayName() const;
    QString sizeDisplayName();
    qint64 filesCount();
private:
    DiskInfo m_diskInfo;

public:
    bool isCanRename() const Q_DECL_OVERRIDE;
    QIcon fileIcon() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;
    DUrl parentUrl() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;
    QSet<MenuAction> disableMenuActionList() const Q_DECL_OVERRIDE;
};

#endif // UDISKDEVICEINFO_H
