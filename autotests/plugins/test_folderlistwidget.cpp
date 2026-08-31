// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_folderlistwidget.cpp
 * @brief Unit tests for FolderListWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/folderlistwidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class FolderListWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FolderListWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FolderListWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FolderListWidgetTest, FolderListWidget)
{
    // Test constructor: FolderListWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FolderListWidgetTest, availableGeometry)
{
    // Test method: QRect availableGeometry((const QPoint &popUpPos))
    QPoint _arg0{};
    auto result = obj->availableGeometry(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FolderListWidgetTest, popUp)
{
    // Test method: void popUp((const QPoint &popupPos))
    QPoint _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->popUp(_arg0));
}
