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
#ifndef DLOCALFILEDEVICE_H
#define DLOCALFILEDEVICE_H

#include "dfm-base/base/dabstractfiledevice.h"


class DFMLocalFileDevicePrivate;
class DFMLocalFileDevice : public DAbstractFileDevice
{
    Q_DECLARE_PRIVATE(DFMLocalFileDevice)

public:

    explicit DFMLocalFileDevice(const QUrl &url);
    ~DFMLocalFileDevice() override;

    bool setFileUrl(const QUrl &url) override;
    bool setFileName(const QString &absoluteFilePath);

    int handle() const override;
    bool resize(qint64 size) override;
    bool flush() override;
    bool syncToDisk(const DAbstractFileDevice::SyncOperate &op = DAbstractFileDevice::DefaultOperate) override;

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
    explicit DFMLocalFileDevice();

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 readLineData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

};



#endif // DLOCALFILEDEVICE_H
