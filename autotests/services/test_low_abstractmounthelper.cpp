// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_abstractmounthelper.cpp
 * @brief Unit tests for AbstractMountHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/mountcontrol/mounthelpers/abstractmounthelper.h"

#include <QTest>

using namespace src;

class AbstractMountHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractMountHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractMountHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractMountHelperTest, AbstractMountHelper)
{
    // Test constructor: AbstractMountHelper(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(AbstractMountHelperTest, checkAuthentication)
{
    // Test bool getter: checkAuthentication()
    bool result = obj->checkAuthentication();
    EXPECT_FALSE(result);

}

TEST_F(AbstractMountHelperTest, AbstractMountHelper_Destructor)
{
    // Test method:  ~AbstractMountHelper(())
    EXPECT_NO_FATAL_FAILURE({ AbstractMountHelper *tmp = new AbstractMountHelper(); delete tmp; });
}
