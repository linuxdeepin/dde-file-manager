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
#ifndef DABSTRACTFILEDEVICE_H
#define DABSTRACTFILEDEVICE_H

#include <dfmglobal.h>

#include <QIODevice>

DFM_BEGIN_NAMESPACE

class DAbstractFileDevicePrivate;
class DAbstractFileDevice : public QIODevice
{
    Q_DECLARE_PRIVATE(DAbstractFileDevice)

public:
    ~DAbstractFileDevice();

    enum FileError {
        NoError
    };

    FileError error() const;

    DUrl fileUrl() const;
    virtual bool setFileUrl(const DUrl &url);

    virtual int handle() const;
    virtual bool resize(qint64 size);
    virtual bool flush();

protected:
    explicit DAbstractFileDevice(QObject *parent = nullptr);
    DAbstractFileDevice(DAbstractFileDevicePrivate &dd, QObject *parent = nullptr);

    void setError(FileError error);

    QScopedPointer<DAbstractFileDevicePrivate> d_ptr;
};

DFM_END_NAMESPACE

#endif // DABSTRACTFILEDEVICE_H
