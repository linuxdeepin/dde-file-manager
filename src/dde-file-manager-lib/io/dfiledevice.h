// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFILEDEVICE_H
#define DFILEDEVICE_H

#include <dfmglobal.h>

#include <QIODevice>

DFM_BEGIN_NAMESPACE

class DFileDevicePrivate;
class DFileDevice : public QIODevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DFileDevice)

public:
    ~DFileDevice();

    DUrl fileUrl() const;

    virtual int handle() const;
    virtual bool resize(qint64 size);
    virtual bool flush();
    virtual bool syncToDisk(bool isVfat = false);
    virtual void closeWriteReadFailed(const bool bwrite);
    virtual void cancelAllOperate();

protected:
    virtual bool setFileUrl(const DUrl &url);

    explicit DFileDevice(QObject *parent = nullptr);
    DFileDevice(DFileDevicePrivate &dd, QObject *parent = nullptr);

    QScopedPointer<DFileDevicePrivate> d_ptr;
};

DFM_END_NAMESPACE

#endif // DFILEDEVICE_H
