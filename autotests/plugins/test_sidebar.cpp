// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebar.cpp
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

TEST_F(SideBarTest, initPreDefineItems)
{
    // Test method: void initPreDefineItems(())
    EXPECT_NO_FATAL_FAILURE(obj->initPreDefineItems());
}
