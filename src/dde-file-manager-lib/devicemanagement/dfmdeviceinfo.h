// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
