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
#include "localfile/dfmlocalfiledevice.h"
#include "private/dfmlocalfiledevice_p.h"

#include <unistd.h>


DFMLocalFileDevicePrivate::DFMLocalFileDevicePrivate(DFMLocalFileDevice *qq)
    : DAbstractFileDevicePrivate(qq)
    , file(new QFile())
{

}

DFMLocalFileDevicePrivate::~DFMLocalFileDevicePrivate()
{
    if (file) {
        file->close();
        delete file;
    }
}

bool DFMLocalFileDevice::setFileUrl(const QUrl &url)
{
    Q_D(DFMLocalFileDevice);

    if (!url.isLocalFile() || !d->file)
        return false;
    QString filename = url.toLocalFile();

    d->file->setFileName(filename);

    return DAbstractFileDevice::setFileUrl(url);
}

bool DFMLocalFileDevice::setFileName(const QString &absoluteFilePath)
{
    return setFileUrl(QUrl::fromLocalFile(absoluteFilePath));
}

int DFMLocalFileDevice::handle() const
{
    Q_D(const DFMLocalFileDevice);

    if (!d->file) {
        return -1;
    }

    return d->file->handle();
}

bool DFMLocalFileDevice::resize(qint64 size)
{
    Q_D(DFMLocalFileDevice);

    if (!d->file) {
        return false;
    }

    return d->file->resize(size);
}

bool DFMLocalFileDevice::flush()
{
    Q_D(DFMLocalFileDevice);

    if (!d->file) {
        return false;
    }

    return d->file->flush();
}

bool DFMLocalFileDevice::syncToDisk(const DAbstractFileDevice::SyncOperate &op)
{
    Q_UNUSED(op);
    Q_D(DFMLocalFileDevice);

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

DFMLocalFileDevice::DFMLocalFileDevice()
    : DAbstractFileDevice(*new DFMLocalFileDevicePrivate(this))
{

}

DFMLocalFileDevice::DFMLocalFileDevice(const QUrl &url)
    : DAbstractFileDevice (url)
{

}

DFMLocalFileDevice::~DFMLocalFileDevice()
{
    close();
}

void DFMLocalFileDevice::setDevice(QIODevice *device)
{
    Q_D(DFMLocalFileDevice);

    d->file = static_cast<QFile*>(device);
}

QIODevice *DFMLocalFileDevice::device() const
{
    Q_D(const DFMLocalFileDevice);

    return d->file;
}

#define CALL_PROXY(fun, ...)\
    const auto && ret = d_func()->file->fun(__VA_ARGS__);\
    setErrorString(d_func()->file->errorString());\
    return ret;

#define CALL_PROXY_CONST(fun, ...)\
    return d_func()->file->fun(__VA_ARGS__);

bool DFMLocalFileDevice::isSequential() const
{
    CALL_PROXY_CONST(isSequential)
}

bool DFMLocalFileDevice::open(QIODevice::OpenMode mode)
{
    Q_D(DFMLocalFileDevice);

    if (d->file) {
        if (!d->file->open(mode)) {
            setErrorString(d->file->errorString());

            return false;
        }
    }

    return DAbstractFileDevice::open(mode);
}

void DFMLocalFileDevice::close()
{
    Q_D(DFMLocalFileDevice);

    DAbstractFileDevice::close();
    if (d->file) {
        d->file->close();
        setErrorString(d->file->errorString());
    }
}

qint64 DFMLocalFileDevice::pos() const
{
    CALL_PROXY_CONST(pos)
}

qint64 DFMLocalFileDevice::size() const
{
    CALL_PROXY_CONST(size)
}

bool DFMLocalFileDevice::seek(qint64 pos)
{
    CALL_PROXY(seek, pos)
}

bool DFMLocalFileDevice::atEnd() const
{
    CALL_PROXY_CONST(atEnd)
}

bool DFMLocalFileDevice::reset()
{
    CALL_PROXY(reset)
}

qint64 DFMLocalFileDevice::bytesAvailable() const
{
    CALL_PROXY_CONST(bytesAvailable)
}

qint64 DFMLocalFileDevice::bytesToWrite() const
{
    CALL_PROXY_CONST(bytesToWrite)
}

bool DFMLocalFileDevice::waitForReadyRead(int msecs)
{
    CALL_PROXY(waitForReadyRead, msecs)
}

bool DFMLocalFileDevice::waitForBytesWritten(int msecs)
{
    CALL_PROXY(waitForBytesWritten, msecs)
}

qint64 DFMLocalFileDevice::readData(char *data, qint64 maxlen)
{
    CALL_PROXY(read, data, maxlen)
}

qint64 DFMLocalFileDevice::readLineData(char *data, qint64 maxlen)
{
    CALL_PROXY(readLine, data, maxlen)
}

qint64 DFMLocalFileDevice::writeData(const char *data, qint64 len)
{
    CALL_PROXY(write, data, len)
}

