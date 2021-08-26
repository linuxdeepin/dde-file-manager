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
#include "private/dabstractfiledevice_p.h"
#include "base/dabstractfiledevice.h"

DAbstractFileDevicePrivate::DAbstractFileDevicePrivate(DAbstractFileDevice *qq)
    : q_ptr(qq)
{

}

DAbstractFileDevicePrivate::~DAbstractFileDevicePrivate()
{

}

DAbstractFileDevice::DAbstractFileDevice(const QUrl &url)
{
    d_ptr->url = url;
}

DAbstractFileDevice::~DAbstractFileDevice()
{

}

QUrl DAbstractFileDevice::url() const
{
    Q_D(const DAbstractFileDevice);

    return d->url;
}

bool DAbstractFileDevice::setFileUrl(const QUrl &url)
{
    Q_D(DAbstractFileDevice);

    d->url = url;

    return true;
}

DAbstractFileDevice::DAbstractFileDevice(DAbstractFileDevicePrivate &dd)
    :d_ptr(&dd)
{

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

bool DAbstractFileDevice::syncToDisk(const DAbstractFileDevice::SyncOperate &op)
{
    Q_UNUSED(op);
    return false;
}

void DAbstractFileDevice::closeWriteReadFailed(const bool bwrite)
{
    Q_UNUSED(bwrite);
}

void DAbstractFileDevice::cancelAllOperate()
{

}
