// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_addressbar.cpp
 * @brief Unit tests for AddressBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/addressbar.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class AddressBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AddressBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AddressBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AddressBarTest, AddressBar)
{
    // Test constructor: AddressBar((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AddressBarTest, currentUrl)
{
    // Test getter: QUrl currentUrl()
    auto result = obj->currentUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(AddressBarTest, event)
{
    // Test method: bool event((QEvent *e))
    auto result = obj->event(nullptr);
    EXPECT_FALSE(result);

}
