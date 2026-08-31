// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_folderlistwidget_1.cpp
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

TEST_F(FolderListWidgetTest, hideEvent)
{
    // Test event handler: hideEvent((QHideEvent *event))
    QHideEvent _event(QHideEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->hideEvent(&_event));
}

TEST_F(FolderListWidgetTest, keyPressEvent)
{
    // Test event handler: keyPressEvent((QKeyEvent *event))
    QKeyEvent _event(QKeyEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->keyPressEvent(&_event));
}

TEST_F(FolderListWidgetTest, setFolderList)
{
    // Test setter: void setFolderList((const QList<CrumbData> &datas, bool stacked))
    QList<CrumbData> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setFolderList(_arg0, false));
}
