// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagmenusceneprivate.cpp
 * @brief Unit tests for TagMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/private/tagmenuscene_p.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagMenuScenePrivateTest, getSurfaceRect)
{
    // Test method: QRect getSurfaceRect((QWidget *w))
    auto result = obj->getSurfaceRect(nullptr);
    EXPECT_FALSE(result.isValid());

}
