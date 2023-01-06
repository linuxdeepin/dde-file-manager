// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATTACHEDDEVICEINTERFACE_H
#define DATTACHEDDEVICEINTERFACE_H

#include <QString>
#include <QPair>
#include <QUrl>

class DAttachedDeviceInterface
{
public:
    virtual bool isValid() = 0; // is device valid and useable
    virtual bool detachable() = 0; // can be unmounted / removable or not.
    virtual void detach() = 0; // do unmount, also do eject if possible.
    virtual QString displayName() = 0; // device display name.
    virtual bool deviceUsageValid() = 0; // storage valid / available or not.
    virtual QPair<quint64, quint64> deviceUsage() = 0; // used / total, in bytes.
    virtual QString iconName() = 0; // device icon (theme) name.
    virtual QUrl mountpointUrl() = 0; // path to the device mount point.
    virtual QUrl accessPointUrl() = 0; // path to the open filemanger.
    virtual ~DAttachedDeviceInterface() = 0;
};

#endif // DATTACHEDDEVICEINTERFACE_H
