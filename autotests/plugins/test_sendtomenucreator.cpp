// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sendtomenucreator.cpp
 * @brief Unit tests for SendToMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/sendtomenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class SendToMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SendToMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SendToMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SendToMenuCreatorTest, create)
{
    // Test getter: DFMBASE_USE_NAMESPACE create()
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });
}

TEST_F(SendToMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
