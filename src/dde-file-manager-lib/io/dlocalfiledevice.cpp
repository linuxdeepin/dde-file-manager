// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
