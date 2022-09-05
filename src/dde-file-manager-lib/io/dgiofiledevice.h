// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DGIOFILEDEVICE_H
#define DGIOFILEDEVICE_H

#include <dfiledevice.h>

DFM_BEGIN_NAMESPACE

class DGIOFileDevicePrivate;
class DGIOFileDevice : public DFileDevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DGIOFileDevice)

public:
    explicit DGIOFileDevice(const DUrl &url, QObject *parent = nullptr);
    ~DGIOFileDevice() override;

    bool setFileUrl(const DUrl &url) override;

    bool open(OpenMode mode) override;
    void close() override;
    int handle() const override;
    qint64 size() const override;
    bool resize(qint64 size) override;
    bool isSequential() const override;
    qint64 pos() const override;
    bool seek(qint64 pos) override;
    bool flush() override;
    bool syncToDisk(bool isVfat = false) override;
    void closeWriteReadFailed(const bool bwrite) override;
    void cancelAllOperate() override;

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
};

DFM_END_NAMESPACE

#endif // DGIOFILEDEVICE_H
