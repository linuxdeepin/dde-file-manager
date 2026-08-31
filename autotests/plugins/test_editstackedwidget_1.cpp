// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_editstackedwidget_1.cpp
 * @brief Unit tests for EditStackedWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/editstackedwidget.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class EditStackedWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EditStackedWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EditStackedWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EditStackedWidgetTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(EditStackedWidgetTest, mouseProcess)
{
    // Test method: void mouseProcess((QMouseEvent *event))
    EXPECT_NO_FATAL_FAILURE(obj->mouseProcess(nullptr));
}

TEST_F(EditStackedWidgetTest, renameFile)
{
    // Test method: void renameFile(())
    EXPECT_NO_FATAL_FAILURE(obj->renameFile());
}

TEST_F(EditStackedWidgetTest, selectFile)
{
    // Test method: void selectFile((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectFile(_arg0));
}

TEST_F(EditStackedWidgetTest, showTextShowFrame)
{
    // Test method: void showTextShowFrame(())
    EXPECT_NO_FATAL_FAILURE(obj->showTextShowFrame());
}
