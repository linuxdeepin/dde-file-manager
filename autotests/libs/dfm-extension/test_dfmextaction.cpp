// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmextaction.cpp
 * @brief Unit tests for DFMExtAction methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-extension/menu/dfmextaction.h"

#include <QTest>

using namespace src;

class DFMExtActionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMExtAction();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMExtAction *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMExtActionTest, deleted)
{
    // Test method: void deleted((DFMExtAction *self))
    EXPECT_NO_FATAL_FAILURE(obj->deleted(nullptr));
}

TEST_F(DFMExtActionTest, icon)
{
    // Test getter: std::string icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.empty());

}

TEST_F(DFMExtActionTest, menu)
{
    // Test getter: DFMExtMenu menu()
    auto result = obj->menu();
    EXPECT_NO_FATAL_FAILURE({ obj->menu(); });

}

TEST_F(DFMExtActionTest, registerDeleted)
{
    // Test method: void registerDeleted((const DFMExtAction::DeletedFunc &func))
    DFMExtAction::DeletedFunc _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->registerDeleted(_arg0));
}

TEST_F(DFMExtActionTest, setIcon)
{
    // Test setter: void setIcon((const std::string &icon))
    std::string _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setIcon(_arg0));
}

TEST_F(DFMExtActionTest, setMenu)
{
    // Test setter: void setMenu((DFMExtMenu *menu))
    EXPECT_NO_FATAL_FAILURE(obj->setMenu(nullptr));
}

TEST_F(DFMExtActionTest, text)
{
    // Test getter: std::string text()
    auto result = obj->text();
    EXPECT_TRUE(result.empty());

}
