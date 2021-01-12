/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
