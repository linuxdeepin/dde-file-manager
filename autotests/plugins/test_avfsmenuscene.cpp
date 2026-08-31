// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfsmenuscene.cpp
 * @brief Unit tests for AvfsMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/avfsmenuscene.h"

#include <QTest>

using namespace dfmplugin_avfsbrowser;

class AvfsMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AvfsMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AvfsMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AvfsMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}
