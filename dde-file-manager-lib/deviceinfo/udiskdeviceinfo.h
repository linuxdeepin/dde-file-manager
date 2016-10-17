#ifndef UDISKDEVICEINFO_H
#define UDISKDEVICEINFO_H

#include "dabstractfileinfo.h"

#include <QDBusInterface>
#include <QString>
#include <QDebug>
#include <QDBusArgument>
#include "dbusinterface/dbustype.h"

class UDiskDeviceInfo : public QObject ,public DAbstractFileInfo
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
        dvd
    };

    UDiskDeviceInfo();
    UDiskDeviceInfo(UDiskDeviceInfo * info);
    UDiskDeviceInfo(const DUrl &url);
    UDiskDeviceInfo(const QString &url);
    UDiskDeviceInfo(const DiskInfo &diskInfo);
    ~UDiskDeviceInfo();
    void setDiskInfo(const DiskInfo &diskInfo);
    DiskInfo getDiskInfo() const;
    QString getId() const;
    QString getName() const;
    QString getType() const;
    QString getPath() const;
    QString getMountPoint() const;
    DUrl getMountPointUrl();
    QString getIcon() const;
    bool canEject() const;
    bool canUnmount() const;
    qulonglong getFree();
    qulonglong getUsed() const;
    qulonglong getTotal();
    qint64 size() const Q_DECL_OVERRIDE;
    QString displayName() const Q_DECL_OVERRIDE;
    MediaType getMediaType() const;
    QString deviceTypeDisplayName() const;
    QString sizeDisplayName() const Q_DECL_OVERRIDE;
    qint64 filesCount() const Q_DECL_OVERRIDE;

    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;

    bool isCanRename() const Q_DECL_OVERRIDE;
    QIcon fileIcon() const Q_DECL_OVERRIDE;
    QIcon fileIcon(int width, int height) const;
    bool isDir() const Q_DECL_OVERRIDE;
    DUrl parentUrl() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;
    QSet<MenuAction> disableMenuActionList() const Q_DECL_OVERRIDE;

    bool exists() const Q_DECL_OVERRIDE;

private:
    DiskInfo m_diskInfo;
};

#endif // UDISKDEVICEINFO_H
