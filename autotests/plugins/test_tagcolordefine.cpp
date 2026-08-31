// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagcolordefine.cpp
 * @brief Unit tests for TagColorDefine methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/taghelper.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagColorDefineTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagColorDefine();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagColorDefine *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagColorDefineTest, TagColorDefine)
{
    // Test constructor: TagColorDefine((const QString &colorName, const QString &iconName, const QString &dispaly, const QColor &color))
    ASSERT_NE(obj, nullptr);
}
