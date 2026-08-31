// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_resetpasswordbyoldpasswordview_1.cpp
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

TEST_F(ResetPasswordByOldPasswordViewTest, btnText)
{
    // Test getter: QStringList btnText()
    auto result = obj->btnText();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ResetPasswordByOldPasswordViewTest, checkInputInfo)
{
    // Test bool getter: checkInputInfo()
    bool result = obj->checkInputInfo();
    EXPECT_FALSE(result);

}

TEST_F(ResetPasswordByOldPasswordViewTest, checkPassword)
{
    // Test method: bool checkPassword((const QString &password))
    QString _arg0{};
    auto result = obj->checkPassword(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ResetPasswordByOldPasswordViewTest, checkRepeatPassword)
{
    // Test bool getter: checkRepeatPassword()
    bool result = obj->checkRepeatPassword();
    EXPECT_FALSE(result);

}

TEST_F(ResetPasswordByOldPasswordViewTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *obj, QEvent *evt))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ResetPasswordByOldPasswordViewTest, onNewPasswordChanged)
{
    // Test method: void onNewPasswordChanged((const QString &pwd))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onNewPasswordChanged(_arg0));
}

TEST_F(ResetPasswordByOldPasswordViewTest, onOldPasswordChanged)
{
    // Test method: void onOldPasswordChanged((const QString &pwd))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onOldPasswordChanged(_arg0));
}

TEST_F(ResetPasswordByOldPasswordViewTest, onPasswordChanged)
{
    // Test method: void onPasswordChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onPasswordChanged());
}

TEST_F(ResetPasswordByOldPasswordViewTest, onRepeatPasswordChanged)
{
    // Test method: void onRepeatPasswordChanged((const QString &pwd))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onRepeatPasswordChanged(_arg0));
}

TEST_F(ResetPasswordByOldPasswordViewTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(ResetPasswordByOldPasswordViewTest, titleText)
{
    // Test getter: QString titleText()
    auto result = obj->titleText();
    EXPECT_TRUE(result.isEmpty());

}
