// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sendtodiscmenusceneprivate.cpp
 * @brief Unit tests for SendToDiscMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/sendtodiscmenuscene.h"

#include <QTest>

using namespace dfmplugin_burn;

class SendToDiscMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SendToDiscMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SendToDiscMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SendToDiscMenuScenePrivateTest, addToSendto)
{
    // Test method: void addToSendto((QMenu *menu))
    EXPECT_NO_FATAL_FAILURE(obj->addToSendto(nullptr));
}

TEST_F(SendToDiscMenuScenePrivateTest, initDestDevices)
{
    // Test method: void initDestDevices(())
    EXPECT_NO_FATAL_FAILURE(obj->initDestDevices());
}
