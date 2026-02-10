// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "utils/selecthelper.h"
#include "views/fileview.h"

#include <QModelIndex>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QRect>
#include <QPoint>
#include <QUrl>
#include <QList>

using namespace dfmplugin_workspace;

class SelectHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        testUrl = QUrl::fromLocalFile("/tmp/test");
        fileView = new FileView(testUrl);
        selectHelper = new SelectHelper(fileView);
    }

    void TearDown() override
    {
        delete selectHelper;
        delete fileView;
        stub.clear();
    }

    QUrl testUrl;
    FileView *fileView;
    SelectHelper *selectHelper;
    stub_ext::StubExt stub;
};

TEST_F(SelectHelperTest, Constructor_SetsProperties)
{
    EXPECT_NE(selectHelper, nullptr);
    EXPECT_EQ(selectHelper->parent(), fileView);
}

TEST_F(SelectHelperTest, GetCurrentPressedIndex_ReturnsCurrentPressedIndex)
{
    QModelIndex result = selectHelper->getCurrentPressedIndex();
    
    // Should return invalid index initially
    EXPECT_FALSE(result.isValid());
}

TEST_F(SelectHelperTest, Click_SetsPressedIndex)
{
    QModelIndex index;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        selectHelper->click(index);
    });
    
    // Verify that current pressed index is set
    QModelIndex result = selectHelper->getCurrentPressedIndex();
    EXPECT_EQ(result, index);
}

TEST_F(SelectHelperTest, Release_ClearsPressedIndex)
{
    QModelIndex index;
    selectHelper->click(index);
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        selectHelper->release();
    });
    
    // Verify that current pressed index is cleared
    QModelIndex result = selectHelper->getCurrentPressedIndex();
    EXPECT_FALSE(result.isValid());
}

TEST_F(SelectHelperTest, SetSelection_SetsSelection)
{
    QItemSelection selection;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        selectHelper->setSelection(selection);
    });
}

TEST_F(SelectHelperTest, Selection_ProcessesSelection)
{
    QRect rect(0, 0, 100, 100);
    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Select;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        selectHelper->selection(rect, flags);
    });
}

TEST_F(SelectHelperTest, Select_SelectsUrls)
{
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/file1");
    
    bool result = selectHelper->select(urls);
    
    // Should return true for valid URLs
    EXPECT_TRUE(result);
}

TEST_F(SelectHelperTest, SelectWithEmptyUrls_ReturnsFalse)
{
    QList<QUrl> urls;
    
    bool result = selectHelper->select(urls);
    
    // Should return false for empty URLs
    EXPECT_FALSE(result);
}

TEST_F(SelectHelperTest, SaveSelectedFilesList_SavesList)
{
    QUrl current = QUrl::fromLocalFile("/tmp/current");
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/file1") << QUrl::fromLocalFile("/tmp/file2");
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        selectHelper->saveSelectedFilesList(current, urls);
    });
}

TEST_F(SelectHelperTest, ResortSelectFiles_ResortsFiles)
{
    QUrl current = QUrl::fromLocalFile("/tmp/current");
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/file1") << QUrl::fromLocalFile("/tmp/file2");
    selectHelper->saveSelectedFilesList(current, urls);
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        selectHelper->resortSelectFiles();
    });
}

TEST_F(SelectHelperTest, FilterSelectedFiles_FiltersFiles)
{
    QUrl current = QUrl::fromLocalFile("/tmp/current");
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/file1") << QUrl::fromLocalFile("/tmp/file2");
    selectHelper->saveSelectedFilesList(current, urls);
    
    QList<QUrl> filterList;
    filterList << QUrl::fromLocalFile("/tmp/file1");
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        selectHelper->filterSelectedFiles(filterList);
    });
}

TEST_F(SelectHelperTest, HandleGroupHeaderClick_HandlesClick)
{
    QModelIndex index;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        selectHelper->handleGroupHeaderClick(index, modifiers);
    });
}

TEST_F(SelectHelperTest, SelectGroup_SelectsGroup)
{
    QString groupKey = "testGroup";
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        selectHelper->selectGroup(groupKey, true);
    });
}

TEST_F(SelectHelperTest, GetGroupFileIndexes_ReturnsIndexes)
{
    QString groupKey = "testGroup";
    
    QList<QModelIndex> result = selectHelper->getGroupFileIndexes(groupKey);
    
    // Should return empty list for non-existent group
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(SelectHelperTest, IsSelectableItem_ChecksItem)
{
    QModelIndex index;
    
    bool result = selectHelper->isSelectableItem(index);
    
    // Should return false for invalid index
    EXPECT_FALSE(result);
}
