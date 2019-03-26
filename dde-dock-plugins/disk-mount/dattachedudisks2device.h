/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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

class DBlockDevice;

class DAttachedUdisks2Device : public DAttachedDeviceInterface
{
public:
    DAttachedUdisks2Device(const DBlockDevice *blockDevicePointer);

    bool isValid() override;
    bool detachable() override;
    void detach() override;
    QString displayName() override;
    bool deviceUsageValid() override;
    QPair<quint64, quint64> deviceUsage() override;
    QString iconName() override;
    QUrl mountpointUrl() override;

    DBlockDevice* blockDevice();

private:
    QScopedPointer<DBlockDevice> c_blockDevice;
    QString deviceDBusId;
    QString mountPoint;

    const QString ddeI18nSym = QStringLiteral("_dde_");
};

#endif // DATTACHEDUDISKS2DEVICE_H
