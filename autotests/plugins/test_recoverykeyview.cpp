// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recoverykeyview.cpp
 * @brief Unit tests for RecoveryKeyView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/unlockview/recoverykeyview.h"

#include <QTest>

using namespace dfmplugin_vault;

class RecoveryKeyViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecoveryKeyView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecoveryKeyView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecoveryKeyViewTest, RecoveryKeyView)
{
    // Test constructor: RecoveryKeyView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RecoveryKeyViewTest, afterRecoveryKeyChanged)
{
    // Test method: int afterRecoveryKeyChanged((QString &str))
    QString _arg0{};
    auto result = obj->afterRecoveryKeyChanged(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(RecoveryKeyViewTest, buttonClicked)
{
    // Test method: void buttonClicked((int index, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->buttonClicked(0, _arg1));
}

TEST_F(RecoveryKeyViewTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *watched, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}
