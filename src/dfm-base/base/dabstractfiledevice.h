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
#ifndef DABSTRACTFILEDEVICE_H
#define DABSTRACTFILEDEVICE_H

#include "dfm-base/dfm_base_global.h"

#include <QIODevice>

class DAbstractFileDevicePrivate;
class DAbstractFileDevice : public QIODevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DAbstractFileDevice)

public:
    enum SyncOperate {
        CloseFileOperate,
        DefaultOperate = 255
    };

    explicit DAbstractFileDevice(const QUrl &url);

    ~DAbstractFileDevice();

    QUrl url() const;

    virtual int handle() const;
    virtual bool resize(qint64 size);
    virtual bool flush();
    virtual bool syncToDisk(const DAbstractFileDevice::SyncOperate &op = DAbstractFileDevice::DefaultOperate);
    virtual void closeWriteReadFailed(const bool bwrite);
    virtual void cancelAllOperate();

protected:
    virtual bool setFileUrl(const QUrl &url);

    QScopedPointer<DAbstractFileDevicePrivate> d_ptr;

    explicit DAbstractFileDevice(DAbstractFileDevicePrivate &dd);
};

#endif // DABSTRACTFILEDEVICE_H
