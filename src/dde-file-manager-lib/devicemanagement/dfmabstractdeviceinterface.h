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


#ifndef DFMABSTRACTDEVICEINTERFACE_H
#define DFMABSTRACTDEVICEINTERFACE_H

#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE

class DFMAbstractDeviceInterface
{
public:
    enum DeviceClassType {
        invalid,
        gvfs,
        udisks2
    };
    virtual void mount() = 0;

    virtual bool unmountable() = 0;
    virtual void unmount() = 0;

    virtual bool ejectable() = 0;
    virtual void eject() = 0;

    virtual bool isReadOnly() const = 0;

    virtual QString name() const = 0; // device original name (can be empty).
    virtual bool canRename() const = 0; // can be renamed or not.
    virtual QString displayName() const = 0; // device display name.
    virtual QString iconName() const = 0; // device icon (theme) name.

    virtual bool deviceUsageValid() const = 0; // storage valid / available or not.
    virtual quint64 availableBytes() const = 0;
    virtual quint64 freeBytes() const = 0; // may NOT equals to `total - available`
    virtual quint64 totalBytes() const = 0;

    virtual QString mountpointPath() const = 0; // "/media/blumia/xxx"

    virtual enum DeviceClassType deviceClassType() { return invalid; } // gio / udisks2, for casting?
    virtual ~DFMAbstractDeviceInterface() = 0;
};

DFM_END_NAMESPACE

#endif // DFMABSTRACTDEVICEINTERFACE_H
