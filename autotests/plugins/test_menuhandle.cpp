// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_menuhandle.cpp
 * @brief Unit tests for MenuHandle methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu.h"

#include <QTest>

using namespace dfmplugin_menu;

class MenuHandleTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MenuHandle();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MenuHandle *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MenuHandleTest, bind)
{
    // Test method: bool bind((const QString &name, const QString &parent))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->bind(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(MenuHandleTest, contains)
{
    // Test method: bool contains((const QString &name))
    QString _arg0{};
    auto result = obj->contains(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(MenuHandleTest, perfectMenuParams)
{
    // Test method: QVariantHash perfectMenuParams((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->perfectMenuParams(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(MenuHandleTest, publishSceneRemoved)
{
    // Test method: void publishSceneRemoved((const QString &scene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->publishSceneRemoved(_arg0));
}
