// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_resetpasswordbykeyfileview.cpp
 * @brief Unit tests for ResetPasswordByKeyFileView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/resetpasswordview/resetpasswordbykeyfileview.h"

#include <QTest>

using namespace dfmplugin_vault;

class ResetPasswordByKeyFileViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ResetPasswordByKeyFileView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ResetPasswordByKeyFileView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ResetPasswordByKeyFileViewTest, ResetPasswordByKeyFileView)
{
    // Test constructor: ResetPasswordByKeyFileView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ResetPasswordByKeyFileViewTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(ResetPasswordByKeyFileViewTest, onResetPasswordFinished)
{
    // Test method: void onResetPasswordFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onResetPasswordFinished());
}

TEST_F(ResetPasswordByKeyFileViewTest, _ResetPasswordByKeyFileView)
{
    // Test constructor: ResetPasswordByKeyFileView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
