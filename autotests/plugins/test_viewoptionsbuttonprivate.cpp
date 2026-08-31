// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewoptionsbuttonprivate.cpp
 * @brief Unit tests for ViewOptionsButtonPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/viewoptionsbutton.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class ViewOptionsButtonPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewOptionsButtonPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewOptionsButtonPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewOptionsButtonPrivateTest, ViewOptionsButtonPrivate)
{
    // Test constructor: ViewOptionsButtonPrivate((ViewOptionsButton *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ViewOptionsButtonPrivateTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(ViewOptionsButtonPrivateTest, popupVisible)
{
    // Test bool getter: popupVisible()
    bool result = obj->popupVisible();
    EXPECT_FALSE(result);

}

TEST_F(ViewOptionsButtonPrivateTest, ViewOptionsButtonPrivate_Destructor)
{
    // Test method:  ~ViewOptionsButtonPrivate(())
    EXPECT_NO_FATAL_FAILURE({ ViewOptionsButtonPrivate *tmp = new ViewOptionsButtonPrivate(); delete tmp; });
}
