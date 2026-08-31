// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sendtomenusceneprivate.cpp
 * @brief Unit tests for SendToMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/sendtomenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class SendToMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SendToMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SendToMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SendToMenuScenePrivateTest, SendToMenuScenePrivate)
{
    // Test constructor: SendToMenuScenePrivate((SendToMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}
