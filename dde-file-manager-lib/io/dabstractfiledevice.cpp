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
#include "dabstractfiledevice.h"
#include "private/dabstractfiledevice_p.h"

DFM_BEGIN_NAMESPACE

DAbstractFileDevice::~DAbstractFileDevice()
{

}

DAbstractFileDevice::FileError DAbstractFileDevice::error() const
{
    Q_D(const DAbstractFileDevice);

    return d->error;
}

DUrl DAbstractFileDevice::fileUrl() const
{
    Q_D(const DAbstractFileDevice);

    return d->url;
}

bool DAbstractFileDevice::setFileUrl(const DUrl &url)
{
    Q_D(DAbstractFileDevice);

    d->url = url;

    return true;
}

int DAbstractFileDevice::handle() const
{
    return -1;
}

bool DAbstractFileDevice::resize(qint64 size)
{
    Q_UNUSED(size)

    return false;
}

bool DAbstractFileDevice::flush()
{
    return false;
}

DAbstractFileDevice::DAbstractFileDevice(QObject *parent)
    : DAbstractFileDevice(*new DAbstractFileDevicePrivate(this), parent)
{

}

DAbstractFileDevice::DAbstractFileDevice(DAbstractFileDevicePrivate &dd, QObject *parent)
    : QIODevice(parent)
    , d_ptr(&dd)
{

}

void DAbstractFileDevice::setError(DAbstractFileDevice::FileError error)
{
    Q_D(DAbstractFileDevice);

    d->error = error;
}

DAbstractFileDevicePrivate::DAbstractFileDevicePrivate(DAbstractFileDevice *qq)
    : q_ptr(qq)
{

}

DFM_END_NAMESPACE
