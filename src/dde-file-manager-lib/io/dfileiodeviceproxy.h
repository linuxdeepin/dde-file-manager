// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
