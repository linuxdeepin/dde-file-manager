// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_urlpushbuttonprivate.cpp
 * @brief Unit tests for UrlPushButtonPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/urlpushbutton.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class UrlPushButtonPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UrlPushButtonPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UrlPushButtonPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UrlPushButtonPrivateTest, arrowWidth)
{
    // Test getter: int arrowWidth()
    auto result = obj->arrowWidth();
    EXPECT_EQ(result, 0);

}

TEST_F(UrlPushButtonPrivateTest, onCompletionCompleted)
{
    // Test method: void onCompletionCompleted(())
    EXPECT_NO_FATAL_FAILURE(obj->onCompletionCompleted());
}
