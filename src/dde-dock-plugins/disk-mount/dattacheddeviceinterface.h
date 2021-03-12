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
