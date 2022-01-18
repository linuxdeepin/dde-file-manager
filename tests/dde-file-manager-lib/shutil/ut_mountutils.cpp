/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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
#include <gtest/gtest.h>

#include "stub-ext/stubext.h"
#include "shutil/mountutils.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <dblockdevice.h>
#include <ddiskmanager.h>

namespace  {
    class TestMountUtils : public testing::Test {
    public:
        void SetUp() override
        {
        }
        void TearDown() override
        {
        }

    public:
    };
}

using namespace stub_ext;

TEST_F(TestMountUtils, tst_isSecurityEnhanceOpen) {
    EXPECT_FALSE(MountUtils::isSecurityEnhanceOpen());

    StubExt stub;
    stub.set_lamda(ADDR(QDBusInterface, isValid), []{ return true; });
    EXPECT_FALSE(MountUtils::isSecurityEnhanceOpen());

    stub.set_lamda(ADDR(QDBusReply<QString>, isValid), []{ return true; });
    stub.set_lamda(ADDR(QDBusReply<QString>, value), []{ return "open"; });
    EXPECT_TRUE(MountUtils::isSecurityEnhanceOpen());
}

TEST_F(TestMountUtils, tst_getSeLinuxMountParam) {
    EXPECT_EQ("", MountUtils::getSeLinuxMountParam());

    StubExt stub;
    stub.set_lamda(MountUtils::isSecurityEnhanceOpen, []{ return true; });
    EXPECT_EQ("", MountUtils::getSeLinuxMountParam());

    stub.set_lamda(ADDR(QDBusInterface, isValid), []{ return true; });
    EXPECT_EQ("", MountUtils::getSeLinuxMountParam());

    stub.set_lamda(ADDR(QDBusReply<QString>, isValid), []{ return true; });
    stub.set_lamda(ADDR(QDBusReply<QString>, value), []{ return "123"; });
    EXPECT_EQ("123", MountUtils::getSeLinuxMountParam());
}

TEST_F(TestMountUtils, tst_getSeLinuxMountParams) {
    EXPECT_TRUE(MountUtils::getSeLinuxMountParams().isEmpty());

    StubExt stub;
    stub.set_lamda(MountUtils::getSeLinuxMountParam, []{ return  "hello world"; });
    EXPECT_NO_FATAL_FAILURE(MountUtils::getSeLinuxMountParams());
}

TEST_F(TestMountUtils, tst_mountBlkWithParams) {
    EXPECT_EQ("", MountUtils::mountBlkWithParams(nullptr));

    StubExt stub;
    stub.set_lamda(ADDR(DBlockDevice, mount), []{ return ""; });

    auto blk = DDiskManager::createBlockDevice("/org/freedesktop/UDisks2/block_devices/loop0");
    if (blk) {
        EXPECT_NO_FATAL_FAILURE(MountUtils::mountBlkWithParams(blk));
        delete blk;
    }
}


