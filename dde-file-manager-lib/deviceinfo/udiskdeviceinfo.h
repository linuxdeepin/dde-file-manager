#ifndef UDISKDEVICEINFO_H
#define UDISKDEVICEINFO_H

#include "dfileinfo.h"

#include <QDBusInterface>
#include <QString>
#include <QDebug>
#include <QDBusArgument>
#include "gvfs/qdiskinfo.h"

class UDiskDeviceInfo;
typedef QExplicitlySharedDataPointer<UDiskDeviceInfo> UDiskDeviceInfoPointer;

class UDiskDeviceInfo : public DFileInfo
{
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
        dvd
    };

    UDiskDeviceInfo();
    UDiskDeviceInfo(UDiskDeviceInfoPointer info);
    UDiskDeviceInfo(const DUrl &url);
    UDiskDeviceInfo(const QString &url);

    ~UDiskDeviceInfo();
    void setDiskInfo(QDiskInfo diskInfo);
    QDiskInfo getDiskInfo() const;
    QString getId() const;
    QString getName() const;
    QString getType() const;
    QString getPath() const;
    QString getMountPoint() const;
    DUrl getMountPointUrl();
    static DUrl getMountPointUrl(QDiskInfo &info);
    QString getIcon() const;
    bool canEject() const;
    bool canUnmount() const;
    qulonglong getFree();
    qulonglong getTotal();
    qint64 size() const Q_DECL_OVERRIDE;
    QString fileDisplayName() const Q_DECL_OVERRIDE;
    MediaType getMediaType() const;
    QString deviceTypeDisplayName() const;
    QString sizeDisplayName() const Q_DECL_OVERRIDE;
    int filesCount() const Q_DECL_OVERRIDE;

    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;

    bool canRename() const Q_DECL_OVERRIDE;
    QIcon fileIcon() const Q_DECL_OVERRIDE;
    QIcon fileIcon(int width, int height) const;
    bool isDir() const Q_DECL_OVERRIDE;
    DUrl parentUrl() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;
    QSet<MenuAction> disableMenuActionList() const Q_DECL_OVERRIDE;

    bool exists() const Q_DECL_OVERRIDE;

private:
    QDiskInfo m_diskInfo;
};

#endif // UDISKDEVICEINFO_H
