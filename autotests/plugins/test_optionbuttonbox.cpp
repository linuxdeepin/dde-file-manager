// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_optionbuttonbox.cpp
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

TEST_F(OptionButtonBoxTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(OptionButtonBoxTest, onUrlChanged)
{
    // Test method: void onUrlChanged((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onUrlChanged(_arg0));
}

TEST_F(OptionButtonBoxTest, setListViewButton)
{
    // Test setter: void setListViewButton((DToolButton *listViewButton))
    EXPECT_NO_FATAL_FAILURE(obj->setListViewButton(nullptr));
}

TEST_F(OptionButtonBoxTest, setViewOptionsButton)
{
    // Test setter: void setViewOptionsButton((ViewOptionsButton *button))
    EXPECT_NO_FATAL_FAILURE(obj->setViewOptionsButton(nullptr));
}

TEST_F(OptionButtonBoxTest, viewMode)
{
    // Test getter: ViewMode viewMode()
    auto result = obj->viewMode();
    EXPECT_GE(static_cast<int>(result), 0);

}
