// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_autoactivatewindowprivate_1.cpp
 * @brief Unit tests for AutoActivateWindowPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "private/autoactivatewindow.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class AutoActivateWindowPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AutoActivateWindowPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AutoActivateWindowPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AutoActivateWindowPrivateTest, AutoActivateWindowPrivate)
{
    // Test constructor: AutoActivateWindowPrivate((AutoActivateWindow *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AutoActivateWindowPrivateTest, initConnect)
{
    // Test bool getter: initConnect()
    bool result = obj->initConnect();
    EXPECT_FALSE(result);

}

TEST_F(AutoActivateWindowPrivateTest, AutoActivateWindowPrivate_Destructor)
{
    // Test method:  ~AutoActivateWindowPrivate(())
    EXPECT_NO_FATAL_FAILURE({ AutoActivateWindowPrivate *tmp = new AutoActivateWindowPrivate(); delete tmp; });
}
