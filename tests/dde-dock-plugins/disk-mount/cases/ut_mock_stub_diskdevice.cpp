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

#include "ut_mock_stub_diskdevice.h"

bool optical_stub()
{
    return true;
}

bool ejectable_stub()
{
    return true;
}

QDBusError lastError_stub()
{
    return QDBusError(QDBusError::Failed,"device is busy");
}

bool removable_stub()
{
    return true;
}

QString dummy_blk_mount_point = ""; //"/media/max_dummy";

QString getDummyMountPoint()
{
    return BLK_DEVICE_MOUNT_POINT;
}

QString getAppRunPath()
{
    if(dummy_blk_mount_point.isEmpty())
        dummy_blk_mount_point = QCoreApplication::applicationDirPath();

    return dummy_blk_mount_point;
}

QByteArrayList mountPoints_stub()
{
    QByteArrayList arraylist;
    arraylist.append(getDummyMountPoint().toUtf8());
    return arraylist;
}

QByteArray device_stub()
{
    return BLK_DEVICE_PATH.toUtf8();
}

bool canPowerOff_stub()
{
    return true;
}

bool isValid_false_stub()
{
    return false;
}

bool isValid_true_stub()
{
    return true;
}

QString idLabel_empty_stub()
{
    return "";
}

QString idLabel_dummy_stub()
{
    return "_dde_dummy";
}

qulonglong size_0_stub()
{
    return 0;
}

qulonglong size_1KB_stub()
{
    return 1024;
}

qulonglong size_2KB_stub()
{
    return 2*1024;
}

QDBusError nolastError_stub()
{
    return QDBusError(QDBusError::NoError,"NoError");
}

bool exists_false_stub()
{
    return false;
}

bool exists_true_stub()
{
    return true;
}

uint getuid_stub()
{
    return 1;
}

uint groupId_stub()
{
    return 2;
}

bool permission_false_stub(QFile::Permissions permissions)
{
    return false;
}

QString owner_stub()
{
    return "max";
}

QString get_empty_string_stub()
{
    return "";
}
