// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfiledevice.h"
#include "private/dfiledevice_p.h"

DFM_BEGIN_NAMESPACE

DFileDevice::~DFileDevice()
{

}

DUrl DFileDevice::fileUrl() const
{
    Q_D(const DFileDevice);

    return d->url;
}

bool DFileDevice::setFileUrl(const DUrl &url)
{
    Q_D(DFileDevice);

    d->url = url;

    return true;
}

int DFileDevice::handle() const
{
    return -1;
}

bool DFileDevice::resize(qint64 size)
{
    Q_UNUSED(size)

    return false;
}

bool DFileDevice::flush()
{
    return false;
}

bool DFileDevice::syncToDisk(bool isVfat)
{
    Q_UNUSED(isVfat);
    return false;
}

void DFileDevice::closeWriteReadFailed(const bool bwrite)
{
    Q_UNUSED(bwrite);
}

void DFileDevice::cancelAllOperate()
{

}

DFileDevice::DFileDevice(QObject *parent)
    : DFileDevice(*new DFileDevicePrivate(this), parent)
{

}

DFileDevice::DFileDevice(DFileDevicePrivate &dd, QObject *parent)
    : QIODevice(parent)
    , d_ptr(&dd)
{

}

DFileDevicePrivate::DFileDevicePrivate(DFileDevice *qq)
    : q_ptr(qq)
{

}

DFileDevicePrivate::~DFileDevicePrivate()
{

}

DFM_END_NAMESPACE
