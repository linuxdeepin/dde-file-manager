// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebar_1.cpp
 * @brief Unit tests for SideBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "sidebar.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(SideBarTest, onAboutToShowSettingDialog)
{
    // Test method: bool onAboutToShowSettingDialog((quint64 winId))
    auto result = obj->onAboutToShowSettingDialog(0);
    EXPECT_FALSE(result);

}

TEST_F(SideBarTest, onWindowClosed)
{
    // Test method: void onWindowClosed((quint64 winId))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowClosed(0));
}

TEST_F(SideBarTest, onWindowOpened)
{
    // Test method: void onWindowOpened((quint64 windId))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowOpened(0));
}

TEST_F(SideBarTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
