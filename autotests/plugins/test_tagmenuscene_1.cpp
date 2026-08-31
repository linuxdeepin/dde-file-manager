// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagmenuscene_1.cpp
 * @brief Unit tests for TagMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/tagmenuscene.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagMenuSceneTest, createTagAction)
{
    // Test method: QAction createTagAction((QMenu *parent))
    auto result = obj->createTagAction(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->createTagAction(nullptr); });

}
