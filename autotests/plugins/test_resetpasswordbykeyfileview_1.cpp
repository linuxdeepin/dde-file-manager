// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_resetpasswordbykeyfileview_1.cpp
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

TEST_F(ResetPasswordByKeyFileViewTest, btnText)
{
    // Test getter: QStringList btnText()
    auto result = obj->btnText();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ResetPasswordByKeyFileViewTest, checkInputInfo)
{
    // Test bool getter: checkInputInfo()
    bool result = obj->checkInputInfo();
    EXPECT_FALSE(result);

}

TEST_F(ResetPasswordByKeyFileViewTest, checkPassword)
{
    // Test method: bool checkPassword((const QString &password))
    QString _arg0{};
    auto result = obj->checkPassword(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ResetPasswordByKeyFileViewTest, checkRepeatPassword)
{
    // Test bool getter: checkRepeatPassword()
    bool result = obj->checkRepeatPassword();
    EXPECT_FALSE(result);

}

TEST_F(ResetPasswordByKeyFileViewTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *obj, QEvent *evt))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ResetPasswordByKeyFileViewTest, onKeyFileSelected)
{
    // Test method: void onKeyFileSelected((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onKeyFileSelected(_arg0));
}

TEST_F(ResetPasswordByKeyFileViewTest, onNewPasswordChanged)
{
    // Test method: void onNewPasswordChanged((const QString &pwd))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onNewPasswordChanged(_arg0));
}

TEST_F(ResetPasswordByKeyFileViewTest, onPasswordChanged)
{
    // Test method: void onPasswordChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onPasswordChanged());
}

TEST_F(ResetPasswordByKeyFileViewTest, onRepeatPasswordChanged)
{
    // Test method: void onRepeatPasswordChanged((const QString &pwd))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onRepeatPasswordChanged(_arg0));
}

TEST_F(ResetPasswordByKeyFileViewTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(ResetPasswordByKeyFileViewTest, titleText)
{
    // Test getter: QString titleText()
    auto result = obj->titleText();
    EXPECT_TRUE(result.isEmpty());

}
