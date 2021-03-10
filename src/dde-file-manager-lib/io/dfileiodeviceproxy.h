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
#ifndef DFILEIODEVICEPROXY_H
#define DFILEIODEVICEPROXY_H

#include <dfiledevice.h>

DFM_BEGIN_NAMESPACE

class DFileIODeviceProxyPrivate;
class DFileIODeviceProxy : public DFileDevice
{
    Q_DECLARE_PRIVATE(DFileIODeviceProxy)

public:
    explicit DFileIODeviceProxy(QIODevice *device = nullptr, QObject *parent = nullptr);
    ~DFileIODeviceProxy() override;

    void setDevice(QIODevice *device);
    QIODevice *device() const;

    bool isSequential() const override;

    bool open(OpenMode mode) override;
    void close() override;

    qint64 pos() const override;
    qint64 size() const override;
    bool seek(qint64 pos) override;
    bool atEnd() const override;
    bool reset() override;

    qint64 bytesAvailable() const override;
    qint64 bytesToWrite() const override;

    bool waitForReadyRead(int msecs) override;
    bool waitForBytesWritten(int msecs) override;

protected:
    DFileIODeviceProxy(DFileIODeviceProxyPrivate &dd, QObject *parent = nullptr);

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 readLineData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
};

DFM_END_NAMESPACE

#endif // DFILEIODEVICEPROXY_H
