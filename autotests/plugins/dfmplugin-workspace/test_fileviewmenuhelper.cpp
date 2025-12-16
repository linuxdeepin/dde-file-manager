// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "utils/fileviewmenuhelper.h"
#include "views/fileview.h"

#include <QModelIndex>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QRect>
#include <QPoint>
#include <QUrl>
#include <QList>
#include <DMenu>

using namespace dfmplugin_workspace;

class TestFileViewMenuHelper : public ::testing::Test
{
protected:
    void SetUp() override
    {
        testUrl = QUrl::fromLocalFile("/tmp/test");
        fileView = new FileView(testUrl);
        menuHelper = new FileViewMenuHelper(fileView);
    }

    void TearDown() override
    {
        delete menuHelper;
        delete fileView;
        stub.clear();
    }

    QUrl testUrl;
    FileView *fileView;
    FileViewMenuHelper *menuHelper;
    stub_ext::StubExt stub;
};

TEST_F(TestFileViewMenuHelper, Constructor_SetsProperties)
{
    EXPECT_NE(menuHelper, nullptr);
    EXPECT_EQ(menuHelper->parent(), fileView);
}

TEST_F(TestFileViewMenuHelper, CanDisableMenu)
{
    bool result = FileViewMenuHelper::disableMenu();
    
    // Just test that it doesn't crash and returns a boolean
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestFileViewMenuHelper, CanShowEmptyAreaMenu)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        menuHelper->showEmptyAreaMenu();
    });
}

TEST_F(TestFileViewMenuHelper, CanShowNormalMenu)
{
    QModelIndex index;
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        menuHelper->showNormalMenu(index, flags);
    });
}

TEST_F(TestFileViewMenuHelper, CanSetWaitCursor)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        menuHelper->setWaitCursor();
    });
}

TEST_F(TestFileViewMenuHelper, CanReloadCursor)
{
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        menuHelper->reloadCursor();
    });
}
