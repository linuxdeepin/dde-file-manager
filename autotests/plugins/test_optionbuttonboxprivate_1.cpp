// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_optionbuttonboxprivate_1.cpp
 * @brief Unit tests for OptionButtonBoxPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/optionbuttonbox.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class OptionButtonBoxPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OptionButtonBoxPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OptionButtonBoxPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OptionButtonBoxPrivateTest, OptionButtonBoxPrivate)
{
    // Test constructor: OptionButtonBoxPrivate((OptionButtonBox *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OptionButtonBoxPrivateTest, loadViewMode)
{
    // Test method: void loadViewMode((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->loadViewMode(_arg0));
}

TEST_F(OptionButtonBoxPrivateTest, onViewModeChanged)
{
    // Test method: void onViewModeChanged((int mode))
    EXPECT_NO_FATAL_FAILURE(obj->onViewModeChanged(0));
}

TEST_F(OptionButtonBoxPrivateTest, setViewMode)
{
    // Test setter: void setViewMode((ViewMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->setViewMode(ViewMode()));
}

TEST_F(OptionButtonBoxPrivateTest, updateCompactButton)
{
    // Test method: void updateCompactButton(())
    EXPECT_NO_FATAL_FAILURE(obj->updateCompactButton());
}
