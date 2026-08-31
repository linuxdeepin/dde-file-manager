// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_backgroundmanagerprivate.cpp
 * @brief Unit tests for BackgroundManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "backgroundmanager.h"

#include <QTest>

using namespace ddplugin_background;

class BackgroundManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BackgroundManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BackgroundManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BackgroundManagerPrivateTest, BackgroundManagerPrivate)
{
    // Test constructor: BackgroundManagerPrivate((BackgroundManager *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BackgroundManagerPrivateTest, isEnableBackground)
{
    // Test bool getter: isEnableBackground()
    bool result = obj->isEnableBackground();
    EXPECT_FALSE(result);

}

TEST_F(BackgroundManagerPrivateTest, relativeGeometry)
{
    // Test getter: QRect relativeGeometry()
    auto result = obj->relativeGeometry();
    EXPECT_FALSE(result.isValid());

}

TEST_F(BackgroundManagerPrivateTest, BackgroundManagerPrivate_Destructor)
{
    // Test method:  ~BackgroundManagerPrivate(())
    EXPECT_NO_FATAL_FAILURE({ BackgroundManagerPrivate *tmp = new BackgroundManagerPrivate(); delete tmp; });
}
