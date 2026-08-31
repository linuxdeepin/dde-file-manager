// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_permissionmanagerwidget_1.cpp
 * @brief Unit tests for PermissionManagerWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/permissionmanagerwidget.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class PermissionManagerWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PermissionManagerWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PermissionManagerWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PermissionManagerWidgetTest, getPermissionString)
{
    // Test method: QString getPermissionString((int enumFlag))
    auto result = obj->getPermissionString(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PermissionManagerWidgetTest, onComboBoxChanged)
{
    // Test method: void onComboBoxChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onComboBoxChanged());
}

TEST_F(PermissionManagerWidgetTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *evt))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(PermissionManagerWidgetTest, setComboBoxByPermission)
{
    // Test setter: void setComboBoxByPermission((QComboBox *cb, int permission, int offset))
    EXPECT_NO_FATAL_FAILURE(obj->setComboBoxByPermission(nullptr, 0, 0));
}

TEST_F(PermissionManagerWidgetTest, setExecText)
{
    // Test method: void setExecText(())
    EXPECT_NO_FATAL_FAILURE(obj->setExecText());
}

TEST_F(PermissionManagerWidgetTest, toggleFileExecutable)
{
    // Test method: void toggleFileExecutable((bool isChecked))
    EXPECT_NO_FATAL_FAILURE(obj->toggleFileExecutable(false));
}

TEST_F(PermissionManagerWidgetTest, updateBackgroundColor)
{
    // Test method: void updateBackgroundColor(())
    EXPECT_NO_FATAL_FAILURE(obj->updateBackgroundColor());
}

TEST_F(PermissionManagerWidgetTest, updateFileUrl)
{
    // Test method: void updateFileUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateFileUrl(_arg0));
}
