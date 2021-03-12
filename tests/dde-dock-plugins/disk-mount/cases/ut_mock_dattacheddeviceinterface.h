/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#pragma once

#include <gmock/gmock.h>
#include <QCoreApplication>

#include "dattacheddeviceinterface.h"

class MockDAttachedDeviceInterface:public DAttachedDeviceInterface{

public:

    MOCK_METHOD0(isValid, bool());
    MOCK_METHOD0(detachable, bool());
    MOCK_METHOD0(detach, void());
    MOCK_METHOD0(displayName, QString());
    MOCK_METHOD0(deviceUsageValid, bool());
    MOCK_METHOD0(deviceUsage, QPair<quint64, quint64>());
    MOCK_METHOD0(iconName, QString());
    MOCK_METHOD0(mountpointUrl, QUrl());
    MOCK_METHOD0(accessPointUrl, QUrl());
};
