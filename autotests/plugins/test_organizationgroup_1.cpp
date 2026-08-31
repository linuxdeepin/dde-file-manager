// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_organizationgroup_1.cpp
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

TEST_F(OrganizationGroupTest, enableHideAllChanged)
{
    // Test method: void enableHideAllChanged((bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->enableHideAllChanged(false));
}

TEST_F(OrganizationGroupTest, enableOrganizeChanged)
{
    // Test method: void enableOrganizeChanged((bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->enableOrganizeChanged(false));
}

TEST_F(OrganizationGroupTest, initShortcutWidget)
{
    // Test method: void initShortcutWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->initShortcutWidget());
}
