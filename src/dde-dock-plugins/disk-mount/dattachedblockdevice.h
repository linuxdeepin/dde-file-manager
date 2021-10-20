/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DATTACHEDBLOCKDEVICE_H
#define DATTACHEDBLOCKDEVICE_H

#include "dattacheddeviceinterface.h"

#include <QUrl>

struct BlockDeviceData
{
    // TODO(zhangs): serallize
};

class DAttachedBlockDevice : public DAttachedDeviceInterface
{
public:
    explicit DAttachedBlockDevice(const QString &json);
    virtual ~DAttachedBlockDevice() override;
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
    void parse();

private:
    QString jsonValue;
    BlockDeviceData data;
};

#endif // DATTACHEDBLOCKDEVICE_H
