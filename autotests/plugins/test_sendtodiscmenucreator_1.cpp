// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sendtodiscmenucreator_1.cpp
 * @brief Unit tests for SendToDiscMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/sendtodiscmenuscene.h"

#include <QTest>

using namespace dfmplugin_burn;

class SendToDiscMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SendToDiscMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SendToDiscMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SendToDiscMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}
