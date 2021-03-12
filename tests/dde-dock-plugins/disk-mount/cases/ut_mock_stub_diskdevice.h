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
#include <dblockdevice.h>
#include <QCoreApplication>
#include <QFile>
#include "ut_mock_stub_common_define.h"

bool optical_stub();

bool ejectable_stub();

bool removable_stub();

QString getDummyMountPoint();

QString getAppRunPath();

QByteArrayList mountPoints_stub();

QByteArray device_stub();

bool canPowerOff_stub();

bool isValid_false_stub();

bool isValid_true_stub();

QString idLabel_empty_stub();

QString idLabel_dummy_stub();

qulonglong size_0_stub();

qulonglong size_1KB_stub();

qulonglong size_2KB_stub();

QDBusError lastError_stub();

QDBusError nolastError_stub();

bool exists_false_stub();

bool exists_true_stub();

uint getuid_stub();

uint groupId_stub();

bool permission_false_stub(QFile::Permissions permissions);

QString owner_stub();

QString get_empty_string_stub();
