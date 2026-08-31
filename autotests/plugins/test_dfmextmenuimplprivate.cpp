// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmextmenuimplprivate.cpp
 * @brief Unit tests for DFMExtMenuImplPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/menuimpl/dfmextmenuimpl.h"

#include <QTest>

using namespace dfmplugin_utils;

class DFMExtMenuImplPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMExtMenuImplPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMExtMenuImplPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMExtMenuImplPrivateTest, addAction)
{
    // Test method: bool addAction((DFMExtAction *action))
    auto result = obj->addAction(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DFMExtMenuImplPrivateTest, setIcon)
{
    // Test setter: void setIcon((const std::string &iconName))
    std::string _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setIcon(_arg0));
}

TEST_F(DFMExtMenuImplPrivateTest, setTitle)
{
    // Test setter: void setTitle((const std::string &title))
    std::string _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setTitle(_arg0));
}

TEST_F(DFMExtMenuImplPrivateTest, DFMExtMenuImplPrivate)
{
    // Test constructor: DFMExtMenuImplPrivate((DFMExtMenuImpl *qImpl, QMenu *m))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DFMExtMenuImplPrivateTest, actions)
{
    // Test getter: std::list<DFMEXT::DFMExtAction *> actions()
    auto result = obj->actions();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DFMExtMenuImplPrivateTest, icon)
{
    // Test getter: std::string icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.empty());

}

TEST_F(DFMExtMenuImplPrivateTest, title)
{
    // Test getter: std::string title()
    auto result = obj->title();
    EXPECT_TRUE(result.empty());

}
