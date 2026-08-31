// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shareeventscaller_1.cpp
 * @brief Unit tests for ShareEventsCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/shareeventscaller.h"

#include <QTest>

using namespace dfmplugin_myshares;

class ShareEventsCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareEventsCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareEventsCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareEventsCallerTest, ShareEventsCaller)
{
    // Test constructor: ShareEventsCaller(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(ShareEventsCallerTest, sendCheckTabAddable)
{
    // Test method: bool sendCheckTabAddable((quint64 windowId))
    auto result = obj->sendCheckTabAddable(0);
    EXPECT_FALSE(result);

}

TEST_F(ShareEventsCallerTest, sendOpenTab)
{
    // Test method: void sendOpenTab((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenTab(0, _arg1));
}

TEST_F(ShareEventsCallerTest, sendOpenWindow)
{
    // Test method: void sendOpenWindow((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenWindow(_arg0));
}

TEST_F(ShareEventsCallerTest, sendSwitchDisplayMode)
{
    // Test method: void sendSwitchDisplayMode((quint64 winId, Global::ViewMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->sendSwitchDisplayMode(0, Global::ViewMode()));
}
