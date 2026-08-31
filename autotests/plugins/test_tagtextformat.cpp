// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagtextformat.cpp
 * @brief Unit tests for TagTextFormat methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/tagtextformat.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagTextFormatTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagTextFormat();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagTextFormat *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagTextFormatTest, borderColor)
{
    // Test getter: QColor borderColor()
    auto result = obj->borderColor();
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagTextFormatTest, colors)
{
    // Test getter: QList<QColor> colors()
    auto result = obj->colors();
    EXPECT_TRUE(result.isEmpty());

}
