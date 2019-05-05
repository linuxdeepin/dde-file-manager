/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    UDiskDeviceInfo(UDiskDeviceInfoPointer info);
    UDiskDeviceInfo(const DUrl &url);
    UDiskDeviceInfo(const QString &url);

    ~UDiskDeviceInfo();
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
    qulonglong getFree();
    qulonglong getTotal();
    qint64 size() const Q_DECL_OVERRIDE;
    QString fileName() const override;
    QString fileDisplayName() const Q_DECL_OVERRIDE;
    MediaType getMediaType() const;
    QString deviceTypeDisplayName() const;
    QString sizeDisplayName() const Q_DECL_OVERRIDE;
    int filesCount() const Q_DECL_OVERRIDE;

    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;

    bool canRename() const override;
    QIcon fileIcon() const Q_DECL_OVERRIDE;
    QIcon fileIcon(int width, int height) const;
    bool isDir() const Q_DECL_OVERRIDE;
    DUrl parentUrl() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;
    QSet<MenuAction> disableMenuActionList() const Q_DECL_OVERRIDE;

    DUrl getUrlByNewFileName(const QString &fileName) const override;
    DUrl getUrlByChildFileName(const QString &fileName) const override;

    bool canRedirectionFileUrl() const Q_DECL_OVERRIDE;
    DUrl redirectedFileUrl() const Q_DECL_OVERRIDE;

    QVariantHash extraProperties() const Q_DECL_OVERRIDE;

    bool exists() const Q_DECL_OVERRIDE;

private:
    QDiskInfo m_diskInfo;
    QString ddeI18nSym = QStringLiteral("_dde_");
};

#endif // UDISKDEVICEINFO_H
