// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "stubext.h"

#include <dfm-base/interfaces/proxyfileinfo.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE

class UT_ProxyFileInfo : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_ProxyFileInfo, bug_195151_nullptrCrashed)
{
    ProxyFileInfo info({});
    EXPECT_NO_FATAL_FAILURE(info.setProxy(nullptr));
    EXPECT_TRUE(info.proxy.isNull());
}
