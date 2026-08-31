// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_urlpushbutton.cpp
 * @brief Unit tests for UrlPushButton methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/urlpushbutton.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class UrlPushButtonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UrlPushButton();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UrlPushButton *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UrlPushButtonTest, isActive)
{
    // Test bool getter: isActive()
    bool result = obj->isActive();
    EXPECT_FALSE(result);

}

TEST_F(UrlPushButtonTest, updateWidth)
{
    // Test method: void updateWidth(())
    EXPECT_NO_FATAL_FAILURE(obj->updateWidth());
}
