/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef ABSTRACTFILEDEVICE_H
#define ABSTRACTFILEDEVICE_H

#include "dfm-base/dfm_base_global.h"

#include <QIODevice>

DFMBASE_BEGIN_NAMESPACE
class AbstractFileDevicePrivate;
class AbstractFileDevice : public QIODevice
{
    Q_OBJECT
    friend class AbstractFileDevicePrivate;

public:
    enum SyncOperate {
        kCloseFileOperate,
        kDefaultOperate = 255
    };
    explicit AbstractFileDevice() = delete;
    explicit AbstractFileDevice(const QUrl &url);
    virtual ~AbstractFileDevice();

    QUrl url() const;
    virtual int handle() const;
    virtual bool resize(qint64 size);
    virtual bool flush();
    virtual bool syncToDisk(const AbstractFileDevice::SyncOperate &op = AbstractFileDevice::kDefaultOperate);

protected:
    virtual bool setFileUrl(const QUrl &url);
    QScopedPointer<AbstractFileDevicePrivate> d;
    explicit AbstractFileDevice(AbstractFileDevicePrivate &dd);
};
DFMBASE_END_NAMESPACE

#endif   // DABSTRACTFILEDEVICE_H
