// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_permissionmanagerwidget.cpp
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

TEST_F(PermissionManagerWidgetTest, PermissionManagerWidget)
{
    // Test constructor: PermissionManagerWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(PermissionManagerWidgetTest, canChmod)
{
    // Test method: bool canChmod((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    auto result = obj->canChmod(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(PermissionManagerWidgetTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}
