// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_newcreatemenusceneprivate.cpp
 * @brief Unit tests for NewCreateMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/newcreatemenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class NewCreateMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NewCreateMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NewCreateMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NewCreateMenuScenePrivateTest, NewCreateMenuScenePrivate)
{
    // Test constructor: NewCreateMenuScenePrivate((NewCreateMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}
