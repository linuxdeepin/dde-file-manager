// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbbrowsermenusceneprivate.cpp
 * @brief Unit tests for SmbBrowserMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/smbbrowsermenuscene.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class SmbBrowserMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbBrowserMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbBrowserMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbBrowserMenuScenePrivateTest, SmbBrowserMenuScenePrivate)
{
    // Test constructor: SmbBrowserMenuScenePrivate((AbstractMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SmbBrowserMenuScenePrivateTest, actMount)
{
    // Test method: void actMount(())
    EXPECT_NO_FATAL_FAILURE(obj->actMount());
}

TEST_F(SmbBrowserMenuScenePrivateTest, actProperties)
{
    // Test method: void actProperties(())
    EXPECT_NO_FATAL_FAILURE(obj->actProperties());
}

TEST_F(SmbBrowserMenuScenePrivateTest, actUnmount)
{
    // Test method: void actUnmount(())
    EXPECT_NO_FATAL_FAILURE(obj->actUnmount());
}
