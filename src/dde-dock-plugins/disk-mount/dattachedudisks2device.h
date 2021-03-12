/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef DATTACHEDUDISKS2DEVICE_H
#define DATTACHEDUDISKS2DEVICE_H

#include "dattacheddeviceinterface.h"

#include <dfmglobal.h>
class ErrorHandleInfc
{
public:
    virtual void onError(DAttachedDeviceInterface *device) = 0;
    virtual ~ErrorHandleInfc() = 0;
};

class DBlockDevice;

class DAttachedUdisks2Device : public DAttachedDeviceInterface
{
public:
    explicit DAttachedUdisks2Device(const DBlockDevice *blockDevicePointer);
    virtual ~DAttachedUdisks2Device() override {}
    bool isValid() override;
    bool detachable() override;
    void detach() override;
    QString displayName() override;
    bool deviceUsageValid() override;
    QPair<quint64, quint64> deviceUsage() override;
    QString iconName() override;
    QUrl mountpointUrl() override;
    QUrl accessPointUrl() override;

    DBlockDevice *blockDevice();
    void setErrorHandler(ErrorHandleInfc *handle)
    {
        errhandle = handle;
    }

private:
    QScopedPointer<DBlockDevice> c_blockDevice;
    QString deviceDBusId;
    QString mountPoint;
    ErrorHandleInfc *errhandle = nullptr;

    const QString ddeI18nSym = QStringLiteral("_dde_");
};

#endif // DATTACHEDUDISKS2DEVICE_H
