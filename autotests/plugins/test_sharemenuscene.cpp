// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharemenuscene.cpp
 * @brief Unit tests for ShareMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/sharemenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ShareMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}
