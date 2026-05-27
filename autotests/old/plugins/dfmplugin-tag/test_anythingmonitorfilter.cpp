// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/utils/anythingmonitorfilter.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/fileutils.h>

#include <gtest/gtest.h>

#include <QFile>

using namespace dfmplugin_tag;
DFMBASE_USE_NAMESPACE

class UT_AnythingMonitorFilter : public testing::Test
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

TEST_F(UT_AnythingMonitorFilter, whetherFilterCurrentPath)
{
    EXPECT_TRUE(ins->whetherFilterCurrentPath("/media"));
    EXPECT_TRUE(!ins->whetherFilterCurrentPath("~/.local/share/Trash"));
}
