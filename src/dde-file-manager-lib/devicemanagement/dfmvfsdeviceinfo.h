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


#ifndef DFMVFSDEVICEINFO_H
#define DFMVFSDEVICEINFO_H

#include "dfmabstractdeviceinterface.h"

#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE

class DFMVfsDevice;
class DFMVfsDeviceInfo : public DFMAbstractDeviceInterface
{
public:
    explicit DFMVfsDeviceInfo(const QUrl mountpointUrl);

    void mount() override;

    bool unmountable() override;
    void unmount() override;

    bool ejectable() override;
    void eject() override;

    bool isReadOnly() const override;

    QString name() const override;
    bool canRename() const override;
    QString displayName() const override; // device display name.
    QString iconName() const override; // device icon (theme) name.

    bool deviceUsageValid() const override; // storage valid / available or not.
    quint64 availableBytes() const override;
    quint64 freeBytes() const override; // may NOT equals to `total - used`
    quint64 totalBytes() const override;

    QString mountpointPath() const override; // "/media/blumia/xxx"

    enum DeviceClassType deviceClassType() override;

    DFMVfsDevice* vfsDevice();
    const DFMVfsDevice* vfsDeviceConst() const;

private:
    QScopedPointer<DFMVfsDevice> c_vfsDevice;
};

DFM_END_NAMESPACE

#endif // DFMVFSDEVICEINFO_H
