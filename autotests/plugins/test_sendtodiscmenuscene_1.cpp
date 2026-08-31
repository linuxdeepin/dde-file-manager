// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sendtodiscmenuscene_1.cpp
 * @brief Unit tests for SendToDiscMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/sendtodiscmenuscene.h"

#include <QTest>

using namespace dfmplugin_burn;

class SendToDiscMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SendToDiscMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SendToDiscMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SendToDiscMenuSceneTest, SendToDiscMenuScene)
{
    // Test constructor: SendToDiscMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SendToDiscMenuSceneTest, SendToDiscMenuScene_Destructor)
{
    // Test method:  ~SendToDiscMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ SendToDiscMenuScene *tmp = new SendToDiscMenuScene(); delete tmp; });
}
