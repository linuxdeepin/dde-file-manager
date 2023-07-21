// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "utils/anythingmonitorfilter.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

class AnythingMonitorFilterTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        ins = &AnythingMonitorFilter::instance();
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    AnythingMonitorFilter *ins;
};

TEST_F(AnythingMonitorFilterTest, whetherFilterCurrentPath)
{
    EXPECT_TRUE(ins->whetherFilterCurrentPath("/media"));
    EXPECT_TRUE(!ins->whetherFilterCurrentPath("~/.local/share/Trash"));
}
