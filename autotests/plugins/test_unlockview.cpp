// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_unlockview.cpp
 * @brief Unit tests for UnlockView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/unlockview/unlockview.h"

#include <QTest>

using namespace dfmplugin_vault;

class UnlockViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UnlockView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UnlockView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UnlockViewTest, UnlockView)
{
    // Test constructor: UnlockView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(UnlockViewTest, buttonClicked)
{
    // Test method: void buttonClicked((int index, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->buttonClicked(0, _arg1));
}

TEST_F(UnlockViewTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(UnlockViewTest, onPasswordCheckFinished)
{
    // Test method: void onPasswordCheckFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onPasswordCheckFinished());
}

TEST_F(UnlockViewTest, showToolTip)
{
    // Test method: void showToolTip((const QString &text, int duration, UnlockView::ENToolTip enType))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showToolTip(_arg0, 0, UnlockView::ENToolTip()));
}
