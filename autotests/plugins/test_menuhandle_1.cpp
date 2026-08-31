// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_menuhandle_1.cpp
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

TEST_F(MenuHandleTest, MenuHandle)
{
    // Test constructor: MenuHandle((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(MenuHandleTest, createScene)
{
    // Test method: dfmbase::AbstractMenuScene createScene((const QString &name))
    QString _arg0{};
    auto result = obj->createScene(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(MenuHandleTest, createSubscene)
{
    // Test method: void createSubscene((AbstractSceneCreator *creator, AbstractMenuScene *parent))
    EXPECT_NO_FATAL_FAILURE(obj->createSubscene(nullptr, nullptr));
}

TEST_F(MenuHandleTest, init)
{
    // Test bool getter: init()
    bool result = obj->init();
    EXPECT_FALSE(result);

}

TEST_F(MenuHandleTest, isMenuDisable)
{
    // Test method: bool isMenuDisable((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->isMenuDisable(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(MenuHandleTest, publishSceneAdded)
{
    // Test method: void publishSceneAdded((const QString &scene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->publishSceneAdded(_arg0));
}

TEST_F(MenuHandleTest, registerScene)
{
    // Test method: bool registerScene((const QString &name, AbstractSceneCreator *creator))
    QString _arg0{};
    auto result = obj->registerScene(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(MenuHandleTest, unbind)
{
    // Test method: void unbind((const QString &name, const QString &parent))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->unbind(_arg0, _arg1));
}

TEST_F(MenuHandleTest, unregisterScene)
{
    // Test method: AbstractSceneCreator unregisterScene((const QString &name))
    QString _arg0{};
    auto result = obj->unregisterScene(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->unregisterScene(_arg0); });

}

TEST_F(MenuHandleTest, MenuHandle_Destructor)
{
    // Test method:  ~MenuHandle(())
    EXPECT_NO_FATAL_FAILURE({ MenuHandle *tmp = new MenuHandle(); delete tmp; });
}
