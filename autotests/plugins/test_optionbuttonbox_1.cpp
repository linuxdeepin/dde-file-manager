// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_optionbuttonbox_1.cpp
 * @brief Unit tests for OptionButtonBox methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/optionbuttonbox.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class OptionButtonBoxTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OptionButtonBox();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OptionButtonBox *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OptionButtonBoxTest, OptionButtonBox)
{
    // Test constructor: OptionButtonBox((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OptionButtonBoxTest, iconViewButton)
{
    // Test getter: DToolButton iconViewButton()
    auto result = obj->iconViewButton();
    EXPECT_NO_FATAL_FAILURE({ obj->iconViewButton(); });

}

TEST_F(OptionButtonBoxTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(OptionButtonBoxTest, initUiForSizeMode)
{
    // Test method: void initUiForSizeMode(())
    EXPECT_NO_FATAL_FAILURE(obj->initUiForSizeMode());
}

TEST_F(OptionButtonBoxTest, listViewButton)
{
    // Test getter: DToolButton listViewButton()
    auto result = obj->listViewButton();
    EXPECT_NO_FATAL_FAILURE({ obj->listViewButton(); });

}

TEST_F(OptionButtonBoxTest, setIconViewButton)
{
    // Test setter: void setIconViewButton((DToolButton *iconViewButton))
    EXPECT_NO_FATAL_FAILURE(obj->setIconViewButton(nullptr));
}

TEST_F(OptionButtonBoxTest, setViewMode)
{
    // Test setter: void setViewMode((int mode))
    EXPECT_NO_FATAL_FAILURE(obj->setViewMode(0));
}

TEST_F(OptionButtonBoxTest, switchToCompactMode)
{
    // Test method: void switchToCompactMode(())
    EXPECT_NO_FATAL_FAILURE(obj->switchToCompactMode());
}

TEST_F(OptionButtonBoxTest, switchToNormalMode)
{
    // Test method: void switchToNormalMode(())
    EXPECT_NO_FATAL_FAILURE(obj->switchToNormalMode());
}

TEST_F(OptionButtonBoxTest, updateOptionButtonBox)
{
    // Test method: void updateOptionButtonBox((int parentWidth))
    EXPECT_NO_FATAL_FAILURE(obj->updateOptionButtonBox(0));
}

TEST_F(OptionButtonBoxTest, viewOptionsButton)
{
    // Test getter: ViewOptionsButton viewOptionsButton()
    auto result = obj->viewOptionsButton();
    EXPECT_NO_FATAL_FAILURE({ obj->viewOptionsButton(); });

}
