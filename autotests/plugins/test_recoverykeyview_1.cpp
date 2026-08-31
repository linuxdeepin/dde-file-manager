// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recoverykeyview_1.cpp
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

TEST_F(RecoveryKeyViewTest, btnText)
{
    // Test getter: QStringList btnText()
    auto result = obj->btnText();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RecoveryKeyViewTest, handleUnlockVault)
{
    // Test method: void handleUnlockVault((bool result))
    EXPECT_NO_FATAL_FAILURE(obj->handleUnlockVault(false));
}

TEST_F(RecoveryKeyViewTest, recoveryKeyChanged)
{
    // Test method: void recoveryKeyChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->recoveryKeyChanged());
}

TEST_F(RecoveryKeyViewTest, showAlertMessage)
{
    // Test method: void showAlertMessage((const QString &text, int duration))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showAlertMessage(_arg0, 0));
}

TEST_F(RecoveryKeyViewTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(RecoveryKeyViewTest, titleText)
{
    // Test getter: QString titleText()
    auto result = obj->titleText();
    EXPECT_TRUE(result.isEmpty());

}
