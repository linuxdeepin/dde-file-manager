// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
