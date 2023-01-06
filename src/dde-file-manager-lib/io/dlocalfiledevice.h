// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DLOCALFILEDEVICE_H
#define DLOCALFILEDEVICE_H

#include <dfileiodeviceproxy.h>

DFM_BEGIN_NAMESPACE

class DLocalFileDevicePrivate;
class DLocalFileDevice : public DFileIODeviceProxy
{
    Q_DECLARE_PRIVATE(DLocalFileDevice)

public:
    explicit DLocalFileDevice(QObject *parent = nullptr);

    bool setFileUrl(const DUrl &url) override;
    bool setFileName(const QString &absoluteFilePath);

    int handle() const override;
    bool resize(qint64 size) override;
    bool flush() override;
    bool syncToDisk(bool isVfat = false) override;

private:
    using DFileIODeviceProxy::setDevice;
};

DFM_END_NAMESPACE

#endif // DLOCALFILEDEVICE_H
