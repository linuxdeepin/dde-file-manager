/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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


#ifndef DFMDEVICEINFO_H
#define DFMDEVICEINFO_H

#include "dfileinfo.h"

DFM_BEGIN_NAMESPACE
class DFMAbstractDeviceInterface;
DFM_END_NAMESPACE

class DFMDeviceInfo;
typedef QExplicitlySharedDataPointer<DFMDeviceInfo> DFMDeviceInfoPointer;

class DFMDeviceInfo : public DFileInfo
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

    DFMDeviceInfo();
    explicit DFMDeviceInfo(const DUrl &url);

    void setUdisks2DeviceInfo(const QString &dbusPath);
    void setVfsDeviceInfo(const DUrl &url);

    bool isValidDevice() const;
    quint64 freeBytes() const;
    quint64 totalBytes() const;
    MediaType mediaType() const;

    bool exists() const override;

    QString fileName() const override;
    QString fileDisplayName() const override;
    bool canRename() const override;

    bool isReadable() const override;
    bool isWritable() const override;

    qint64 size() const override;
    int filesCount() const override;
    QIcon fileIcon() const override;

    bool isDir() const override;
    DUrl parentUrl() const override;

    bool canRedirectionFileUrl() const override;
    DUrl redirectedFileUrl() const override;

    DFM_NAMESPACE::DFMAbstractDeviceInterface* attachedDevice();
    const DFM_NAMESPACE::DFMAbstractDeviceInterface* attachedDeviceConst() const;

private:
    QScopedPointer<DFM_NAMESPACE::DFMAbstractDeviceInterface> c_attachedDevice;
};

#endif // DFMDEVICEINFO_H
