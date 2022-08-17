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

#include "stubext.h"
#include "plugins/common/dfmplugin-burn/utils/burnhelper.h"
#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DPBURN_USE_NAMESPACE

class UT_BurnHelper : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_BurnHelper, IsBurnEnabled)
{
    bool ret { false };
    stub.set_lamda(&DConfigManager::value, [&] { __DBG_STUB_INVOKE__ return ret; });
    EXPECT_FALSE(BurnHelper::isBurnEnabled());

    ret = true;
    EXPECT_TRUE(BurnHelper::isBurnEnabled());
}

// TEST_F(UT_BurnHelper, ){}
