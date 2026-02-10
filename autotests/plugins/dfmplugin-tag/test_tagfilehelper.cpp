// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/tagfilehelper.h"

#include <gtest/gtest.h>

#include <QUrl>
#include <QRect>

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
