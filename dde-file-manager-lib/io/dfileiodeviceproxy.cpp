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
#include "dfileiodeviceproxy.h"
#include "private/dfileiodeviceproxy_p.h"

DFM_BEGIN_NAMESPACE

DFileIODeviceProxyPrivate::DFileIODeviceProxyPrivate(DFileIODeviceProxy *qq)
    : DAbstractFileDevicePrivate(qq)
{

}

DFileIODeviceProxy::DFileIODeviceProxy(QIODevice *device, QObject *parent)
    : DFileIODeviceProxy(*new  DFileIODeviceProxyPrivate(this), parent)
{
    d_func()->device = device;
}

void DFileIODeviceProxy::setDevice(QIODevice *device)
{
    Q_D(DFileIODeviceProxy);

    d->device = device;
}

QIODevice *DFileIODeviceProxy::device() const
{
    Q_D(const DFileIODeviceProxy);

    return d->device;
}

#define CALL_PROXY(fun, ...)\
    if (d_func()->device) return d_func()->device->fun(__VA_ARGS__);\
    return DAbstractFileDevice::fun(__VA_ARGS__);

bool DFileIODeviceProxy::isSequential() const
{
    CALL_PROXY(isSequential)
}

bool DFileIODeviceProxy::open(QIODevice::OpenMode mode)
{
    CALL_PROXY(open, mode)
}

void DFileIODeviceProxy::close()
{
    CALL_PROXY(close)
}

qint64 DFileIODeviceProxy::pos() const
{
    CALL_PROXY(pos)
}

qint64 DFileIODeviceProxy::size() const
{
    CALL_PROXY(size)
}

bool DFileIODeviceProxy::seek(qint64 pos)
{
    CALL_PROXY(seek, pos)
}

bool DFileIODeviceProxy::atEnd() const
{
    CALL_PROXY(atEnd)
}

bool DFileIODeviceProxy::reset()
{
    CALL_PROXY(reset)
}

qint64 DFileIODeviceProxy::bytesAvailable() const
{
    CALL_PROXY(bytesAvailable)
}

qint64 DFileIODeviceProxy::bytesToWrite() const
{
    CALL_PROXY(bytesToWrite)
}

bool DFileIODeviceProxy::waitForReadyRead(int msecs)
{
    CALL_PROXY(waitForReadyRead, msecs)
}

bool DFileIODeviceProxy::waitForBytesWritten(int msecs)
{
    CALL_PROXY(waitForBytesWritten, msecs)
}

DFileIODeviceProxy::DFileIODeviceProxy(DFileIODeviceProxyPrivate &dd, QObject *parent)
    : DAbstractFileDevice(dd, parent)
{

}

qint64 DFileIODeviceProxy::readData(char *data, qint64 maxlen)
{
    Q_D(DFileIODeviceProxy);

    if (d->device)
        return d->device->read(data, maxlen);

    return -1;
}

qint64 DFileIODeviceProxy::readLineData(char *data, qint64 maxlen)
{
    Q_D(DFileIODeviceProxy);

    if (d->device)
        return d->device->readLine(data, maxlen);

    return -1;
}

qint64 DFileIODeviceProxy::writeData(const char *data, qint64 len)
{
    Q_D(DFileIODeviceProxy);

    if (d->device)
        return d->device->write(data, len);

    return -1;
}

DFM_END_NAMESPACE
