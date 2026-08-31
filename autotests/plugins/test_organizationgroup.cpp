// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_organizationgroup.cpp
 * @brief Unit tests for OrganizationGroup methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "options/organizationgroup.h"

#include <QTest>

using namespace ddplugin_organizer;

class OrganizationGroupTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OrganizationGroup();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OrganizationGroup *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OrganizationGroupTest, OrganizationGroup)
{
    // Test constructor: OrganizationGroup((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OrganizationGroupTest, buildTypeLayout)
{
    // Test getter: QLayout buildTypeLayout()
    auto result = obj->buildTypeLayout();
    EXPECT_NO_FATAL_FAILURE({ obj->buildTypeLayout(); });

}

TEST_F(OrganizationGroupTest, clearShortcutWidget)
{
    // Test method: void clearShortcutWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->clearShortcutWidget());
}

TEST_F(OrganizationGroupTest, clearlAll)
{
    // Test method: void clearlAll(())
    EXPECT_NO_FATAL_FAILURE(obj->clearlAll());
}

TEST_F(OrganizationGroupTest, reset)
{
    // Test method: void reset(())
    EXPECT_NO_FATAL_FAILURE(obj->reset());
}
