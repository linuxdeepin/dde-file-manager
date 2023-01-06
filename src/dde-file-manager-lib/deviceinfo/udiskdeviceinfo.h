// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    enum MediaType {
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
    explicit UDiskDeviceInfo(UDiskDeviceInfoPointer info);
    explicit UDiskDeviceInfo(const DUrl &url);
    explicit UDiskDeviceInfo(const QString &url);

    ~UDiskDeviceInfo() override;
    void setDiskInfo(QDiskInfo diskInfo);
    QDiskInfo getDiskInfo() const;
    QString getId() const;
    QString getIdType() const;
    QString getName() const;
    QString getType() const;
    QString getPath() const;
    QString getDBusPath() const;
    QString getMountPoint() const;
    DUrl getMountPointUrl() const;
    QString getIcon() const;
    bool canEject() const;
    bool canStop() const;
    bool canUnmount() const;
    bool optical() const;
    bool opticalBlank() const;
    bool opticalReuseable() const;
    qulonglong getFree();
    qulonglong getTotal();
    qint64 size() const override;
    QString fileName() const override;
    QString fileDisplayName() const override;
    MediaType getMediaType() const;
    QString deviceTypeDisplayName() const;
    QString sizeDisplayName() const override;
    int filesCount() const override;

    bool isReadable() const override;
    bool isWritable() const override;

    bool canRename() const override;
    QIcon fileIcon() const override;
    QIcon fileIcon(int width, int height) const;
    bool isDir() const override;
    DUrl parentUrl() const override;

    QVector<MenuAction> menuActionList(MenuType type) const override;
    QSet<MenuAction> disableMenuActionList() const override;

    DUrl getUrlByNewFileName(const QString &fileName) const override;
    DUrl getUrlByChildFileName(const QString &fileName) const override;

    bool canRedirectionFileUrl() const override;
    DUrl redirectedFileUrl() const override;

    QVariantHash extraProperties() const override;

    bool exists() const override;

private:
    QDiskInfo m_diskInfo;
    QString ddeI18nSym = QStringLiteral("_dde_");
};

#endif // UDISKDEVICEINFO_H
