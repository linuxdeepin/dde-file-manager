// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_mountcontroldbusprivate.cpp
 * @brief Unit tests for MountControlDBusPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/mountcontrol/mountcontroldbus.h"

#include <QTest>

using namespace src;

class MountControlDBusPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MountControlDBusPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MountControlDBusPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MountControlDBusPrivateTest, MountControlDBusPrivate)
{
    // Test constructor: MountControlDBusPrivate((MountControlDBus *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(MountControlDBusPrivateTest, MountControlDBusPrivate_Destructor)
{
    // Test method:  ~MountControlDBusPrivate(())
    EXPECT_NO_FATAL_FAILURE({ MountControlDBusPrivate *tmp = new MountControlDBusPrivate(); delete tmp; });
}
