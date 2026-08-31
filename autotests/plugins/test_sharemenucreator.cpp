// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharemenucreator.cpp
 * @brief Unit tests for ShareMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/sharemenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ShareMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareMenuCreatorTest, create)
{
    // Test getter: dfmbase::AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(ShareMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
