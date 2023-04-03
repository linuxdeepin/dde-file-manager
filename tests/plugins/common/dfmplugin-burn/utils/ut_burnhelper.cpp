// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-burn/utils/burnhelper.h"
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

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
