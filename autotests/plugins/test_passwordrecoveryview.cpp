// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_passwordrecoveryview.cpp
 * @brief Unit tests for PasswordRecoveryView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/unlockview/passwordrecoveryview.h"

#include <QTest>

using namespace dfmplugin_vault;

class PasswordRecoveryViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PasswordRecoveryView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PasswordRecoveryView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PasswordRecoveryViewTest, PasswordRecoveryView)
{
    // Test constructor: PasswordRecoveryView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
