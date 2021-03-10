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
