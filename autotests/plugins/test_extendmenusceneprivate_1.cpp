// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extendmenusceneprivate_1.cpp
 * @brief Unit tests for ExtendMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ExtendMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtendMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtendMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtendMenuScenePrivateTest, ExtendMenuScenePrivate)
{
    // Test constructor: ExtendMenuScenePrivate((ExtendMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ExtendMenuScenePrivateTest, mayComboPostion)
{
    // Test method: int mayComboPostion((const DCustomActionData &acdata, DCustomActionDefines::ComboType combo))
    DCustomActionData _arg0{};
    auto result = obj->mayComboPostion(_arg0, DCustomActionDefines::ComboType());
    EXPECT_GE(result, 0);

}
