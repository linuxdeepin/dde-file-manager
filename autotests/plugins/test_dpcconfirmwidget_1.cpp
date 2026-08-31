// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dpcconfirmwidget_1.cpp
 * @brief Unit tests for DPCConfirmWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dialogs/dpcwidget/dpcconfirmwidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class DPCConfirmWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DPCConfirmWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DPCConfirmWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DPCConfirmWidgetTest, DPCConfirmWidget)
{
    // Test constructor: DPCConfirmWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DPCConfirmWidgetTest, checkNewPassword)
{
    // Test bool getter: checkNewPassword()
    bool result = obj->checkNewPassword();
    EXPECT_FALSE(result);

}

TEST_F(DPCConfirmWidgetTest, checkRepeatPassword)
{
    // Test bool getter: checkRepeatPassword()
    bool result = obj->checkRepeatPassword();
    EXPECT_FALSE(result);

}

TEST_F(DPCConfirmWidgetTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(DPCConfirmWidgetTest, initLibrary)
{
    // Test method: void initLibrary(())
    EXPECT_NO_FATAL_FAILURE(obj->initLibrary());
}

TEST_F(DPCConfirmWidgetTest, onEditingFinished)
{
    // Test method: void onEditingFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onEditingFinished());
}

TEST_F(DPCConfirmWidgetTest, onPasswdChanged)
{
    // Test method: void onPasswdChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onPasswdChanged());
}

TEST_F(DPCConfirmWidgetTest, setEnabled)
{
    // Test setter: void setEnabled((bool enabled))
    EXPECT_NO_FATAL_FAILURE(obj->setEnabled(false));
}

TEST_F(DPCConfirmWidgetTest, showToolTips)
{
    // Test method: void showToolTips((const QString &msg, DPasswordEdit *pwdEdit))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showToolTips(_arg0, nullptr));
}
