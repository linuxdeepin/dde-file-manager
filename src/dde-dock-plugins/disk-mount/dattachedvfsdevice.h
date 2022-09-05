// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATTACHEDVFSDEVICE_H
#define DATTACHEDVFSDEVICE_H

#include "dattacheddeviceinterface.h"

#include <QUrl>
#include <QScopedPointer>

#include <dfmglobal.h>
#include <dgiomount.h>

class DAttachedVfsDevice : public DAttachedDeviceInterface
{
public:
    explicit DAttachedVfsDevice(const QString &mountpointPath);
    virtual ~DAttachedVfsDevice() override {}
    bool isValid() override;
    bool detachable() override;
    void detach() override;
    QString displayName() override;
    bool deviceUsageValid() override;
    QPair<quint64, quint64> deviceUsage() override;
    QString iconName() override;
    QUrl mountpointUrl() override;
    QUrl accessPointUrl() override;

private:
    QScopedPointer<DGioMount> m_dgioMount;
    QString m_mountpointPath;
};

#endif // DATTACHEDVFSDEVICE_H
