// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/utils/tagtextformat.h"

#include <gtest/gtest.h>

#include <QColor>

using namespace dfmplugin_tag;

class TagTextFormatTest : public testing::Test
{

protected:
    virtual void SetUp() override
    {
        ins = new TagTextFormat(1, QList<QColor>() << QColor("red"), QColor("red"));
    }
    virtual void TearDown() override
    {
        stub.clear();
        if (ins) {
            delete ins;
            ins = nullptr;
        }
    }

private:
    stub_ext::StubExt stub;
    TagTextFormat *ins;
};

TEST_F(TagTextFormatTest, colors)
{
    EXPECT_TRUE(ins->colors().contains(QColor("red")));
}

TEST_F(TagTextFormatTest, borderColor)
{
    EXPECT_TRUE(ins->borderColor() == QColor("red"));
}

TEST_F(TagTextFormatTest, diameter)
{
    EXPECT_TRUE(ins->diameter() == kTagDiameter);
}
