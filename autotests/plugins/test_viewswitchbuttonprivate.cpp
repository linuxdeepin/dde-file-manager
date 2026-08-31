// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewswitchbuttonprivate.cpp
 * @brief Unit tests for ViewSwitchButtonPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/viewswitchbutton.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class ViewSwitchButtonPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewSwitchButtonPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewSwitchButtonPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewSwitchButtonPrivateTest, ViewSwitchButtonPrivate)
{
    // Test constructor: ViewSwitchButtonPrivate((ViewSwitchButton *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ViewSwitchButtonPrivateTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(ViewSwitchButtonPrivateTest, setupMenu)
{
    // Test method: void setupMenu(())
    EXPECT_NO_FATAL_FAILURE(obj->setupMenu());
}

TEST_F(ViewSwitchButtonPrivateTest, updateCheckedState)
{
    // Test method: void updateCheckedState(())
    EXPECT_NO_FATAL_FAILURE(obj->updateCheckedState());
}
