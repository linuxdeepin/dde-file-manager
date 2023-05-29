// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "utils/tagfilehelper.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/clipboard.h>

#include <gtest/gtest.h>

#include <DCrumbEdit>
#include <QPaintEvent>
#include <QPainter>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

class TagFileHelperTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        ins = TagFileHelper::instance();
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    TagFileHelper *ins;
};

TEST_F(TagFileHelperTest, openFileInPlugin)
{
    EXPECT_TRUE(!ins->openFileInPlugin(1, QList<QUrl>() << QUrl("file:/test")));
    EXPECT_TRUE(!ins->openFileInPlugin(1, QList<QUrl>()));
    EXPECT_TRUE(ins->openFileInPlugin(1, QList<QUrl>() << QUrl("tag:/test")));
}
