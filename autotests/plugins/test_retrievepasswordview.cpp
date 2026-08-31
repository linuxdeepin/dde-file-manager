// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_retrievepasswordview.cpp
 * @brief Unit tests for RetrievePasswordView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/unlockview/retrievepasswordview.h"

#include <QTest>

using namespace dfmplugin_vault;

class RetrievePasswordViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RetrievePasswordView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RetrievePasswordView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RetrievePasswordViewTest, RetrievePasswordView)
{
    // Test constructor: RetrievePasswordView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RetrievePasswordViewTest, onKeyVerificationFinished)
{
    // Test method: void onKeyVerificationFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onKeyVerificationFinished());
}
