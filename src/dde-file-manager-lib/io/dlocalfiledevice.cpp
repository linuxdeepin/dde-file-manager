/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "dlocalfiledevice.h"
#include "private/dlocalfiledevice_p.h"

#include <unistd.h>

DFM_BEGIN_NAMESPACE

DLocalFileDevicePrivate::DLocalFileDevicePrivate(DLocalFileDevice *qq)
    : DFileIODeviceProxyPrivate(qq)
    , file(new QFile())
{
}

DLocalFileDevicePrivate::~DLocalFileDevicePrivate()
{
    if (file) {
        file->close();
        delete file;
    }
}

DLocalFileDevice::DLocalFileDevice(QObject *parent)
    : DFileIODeviceProxy(*new DLocalFileDevicePrivate(this), parent)
{
    setDevice(d_func()->file);
}

bool DLocalFileDevice::setFileUrl(const DUrl &url)
{
    Q_D(DLocalFileDevice);

    if (!url.isLocalFile() || !d->file)
        return false;
    QString filename = url.toLocalFile();

    d->file->setFileName(filename);

    return DFileDevice::setFileUrl(url);
}

bool DLocalFileDevice::setFileName(const QString &absoluteFilePath)
{
    return setFileUrl(DUrl::fromLocalFile(absoluteFilePath));
}

int DLocalFileDevice::handle() const
{
    Q_D(const DLocalFileDevice);

    if (!d->file) {
        return -1;
    }

    return d->file->handle();
}

bool DLocalFileDevice::resize(qint64 size)
{
    Q_D(DLocalFileDevice);

    if (!d->file) {
        return false;
    }

    return d->file->resize(size);
}

bool DLocalFileDevice::flush()
{
    Q_D(DLocalFileDevice);

    if (!d->file) {
        return false;
    }

    return d->file->flush();
}

bool DLocalFileDevice::syncToDisk(bool isVfat)
{
    Q_UNUSED(isVfat);
    Q_D(DLocalFileDevice);

    if (!d->file) {
        return false;
    }

    int ret = fdatasync(d->file->handle());

    if (ret != 0) {
        setErrorString(QString::fromLocal8Bit(strerror(errno)));

        return false;
    }

    return true;
}

DFM_END_NAMESPACE
