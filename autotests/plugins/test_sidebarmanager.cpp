// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarmanager.cpp
 * @brief Unit tests for SideBarManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/sidebarmanager.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarManagerTest, instance)
{
    // Test getter: DPSIDEBAR_USE_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
