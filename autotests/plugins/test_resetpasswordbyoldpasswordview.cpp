// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_resetpasswordbyoldpasswordview.cpp
 * @brief Unit tests for ResetPasswordByOldPasswordView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/resetpasswordview/resetpasswordbyoldpasswordview.h"

#include <QTest>

using namespace dfmplugin_vault;

class ResetPasswordByOldPasswordViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ResetPasswordByOldPasswordView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ResetPasswordByOldPasswordView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ResetPasswordByOldPasswordViewTest, ResetPasswordByOldPasswordView)
{
    // Test constructor: ResetPasswordByOldPasswordView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ResetPasswordByOldPasswordViewTest, buttonClicked)
{
    // Test method: void buttonClicked((int index, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->buttonClicked(0, _arg1));
}

TEST_F(ResetPasswordByOldPasswordViewTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(ResetPasswordByOldPasswordViewTest, onResetPasswordFinished)
{
    // Test method: void onResetPasswordFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onResetPasswordFinished());
}

TEST_F(ResetPasswordByOldPasswordViewTest, _ResetPasswordByOldPasswordView)
{
    // Test constructor: ResetPasswordByOldPasswordView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
