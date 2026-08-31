// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_organizerutils_1.cpp
 * @brief Unit tests for OrganizerUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/organizerutils.h"

#include <QTest>

using namespace ddplugin_organizer;

class OrganizerUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OrganizerUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OrganizerUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OrganizerUtilsTest, OrganizerUtils)
{
    // Test constructor: OrganizerUtils(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(OrganizerUtilsTest, isAllItemCategory)
{
    // Test method: bool isAllItemCategory((const ItemCategories &flags))
    ItemCategories _arg0{};
    auto result = obj->isAllItemCategory(_arg0);
    EXPECT_FALSE(result);

}
