// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mysharemenusceneprivate.cpp
 * @brief Unit tests for MyShareMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/mysharemenuscene.h"

#include <QTest>

using namespace dfmplugin_myshares;

class MyShareMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MyShareMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MyShareMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MyShareMenuScenePrivateTest, MyShareMenuScenePrivate)
{
    // Test constructor: MyShareMenuScenePrivate((AbstractMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(MyShareMenuScenePrivateTest, createFileMenu)
{
    // Test method: void createFileMenu((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->createFileMenu(nullptr));
}

TEST_F(MyShareMenuScenePrivateTest, triggered)
{
    // Test method: bool triggered((const QString &id))
    QString _arg0{};
    auto result = obj->triggered(_arg0);
    EXPECT_FALSE(result);

}
