// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_oemmenuscene.cpp
 * @brief Unit tests for OemMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "oemmenuscene/oemmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class OemMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OemMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OemMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OemMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(OemMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}
