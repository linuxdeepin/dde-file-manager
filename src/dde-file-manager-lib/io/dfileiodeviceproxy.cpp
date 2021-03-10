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
#include "dfileiodeviceproxy.h"
#include "private/dfileiodeviceproxy_p.h"

DFM_BEGIN_NAMESPACE

DFileIODeviceProxyPrivate::DFileIODeviceProxyPrivate(DFileIODeviceProxy *qq)
    : DFileDevicePrivate(qq)
{

}

DFileIODeviceProxy::DFileIODeviceProxy(QIODevice *device, QObject *parent)
    : DFileIODeviceProxy(*new  DFileIODeviceProxyPrivate(this), parent)
{
    d_func()->device = device;
}

DFileIODeviceProxy::~DFileIODeviceProxy()
{
    close();
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
    const auto && ret = d_func()->device->fun(__VA_ARGS__);\
    setErrorString(d_func()->device->errorString());\
    return ret;

#define CALL_PROXY_CONST(fun, ...)\
    return d_func()->device->fun(__VA_ARGS__);

bool DFileIODeviceProxy::isSequential() const
{
    CALL_PROXY_CONST(isSequential)
}

bool DFileIODeviceProxy::open(QIODevice::OpenMode mode)
{
    Q_D(DFileIODeviceProxy);

    if (d->device) {
        if (!d->device->open(mode)) {
            setErrorString(d->device->errorString());

            return false;
        }
    }

    return DFileDevice::open(mode);
}

void DFileIODeviceProxy::close()
{
    Q_D(DFileIODeviceProxy);

    DFileDevice::close();
    if (d->device) {
        d->device->close();
        setErrorString(d->device->errorString());
    }
}

qint64 DFileIODeviceProxy::pos() const
{
    CALL_PROXY_CONST(pos)
}

qint64 DFileIODeviceProxy::size() const
{
    CALL_PROXY_CONST(size)
}

bool DFileIODeviceProxy::seek(qint64 pos)
{
    CALL_PROXY(seek, pos)
}

bool DFileIODeviceProxy::atEnd() const
{
    CALL_PROXY_CONST(atEnd)
}

bool DFileIODeviceProxy::reset()
{
    CALL_PROXY(reset)
}

qint64 DFileIODeviceProxy::bytesAvailable() const
{
    CALL_PROXY_CONST(bytesAvailable)
}

qint64 DFileIODeviceProxy::bytesToWrite() const
{
    CALL_PROXY_CONST(bytesToWrite)
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
    : DFileDevice(dd, parent)
{

}

qint64 DFileIODeviceProxy::readData(char *data, qint64 maxlen)
{
    CALL_PROXY(read, data, maxlen)
}

qint64 DFileIODeviceProxy::readLineData(char *data, qint64 maxlen)
{
    CALL_PROXY(readLine, data, maxlen)
}

qint64 DFileIODeviceProxy::writeData(const char *data, qint64 len)
{
    CALL_PROXY(write, data, len)
}

DFM_END_NAMESPACE
