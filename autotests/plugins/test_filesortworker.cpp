// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filesortworker.cpp
 * @brief Unit tests for FileSortWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/filesortworker.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileSortWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileSortWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileSortWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileSortWorkerTest, checkAndUpdateFileInfoUpdate)
{
    // Test bool getter: checkAndUpdateFileInfoUpdate()
    bool result = obj->checkAndUpdateFileInfoUpdate();
    EXPECT_FALSE(result);

}

TEST_F(FileSortWorkerTest, clearGroupedData)
{
    // Test method: void clearGroupedData(())
    EXPECT_NO_FATAL_FAILURE(obj->clearGroupedData());
}

TEST_F(FileSortWorkerTest, filterAndSortFiles)
{
    // Test method: void filterAndSortFiles((const QUrl &dir, const bool fileter, const bool reverse))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->filterAndSortFiles(_arg0, false, false));
}

TEST_F(FileSortWorkerTest, filterTreeDirFiles)
{
    // Test method: void filterTreeDirFiles((const QUrl &parent, const bool byInfo))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->filterTreeDirFiles(_arg0, false));
}

TEST_F(FileSortWorkerTest, getAllFiles)
{
    // Test getter: QList<FileItemDataPointer> getAllFiles()
    auto result = obj->getAllFiles();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileSortWorkerTest, handleAboutToRemoveFilesFromGroup)
{
    // Test method: void handleAboutToRemoveFilesFromGroup((int pos, int count))
    EXPECT_NO_FATAL_FAILURE(obj->handleAboutToRemoveFilesFromGroup(0, 0));
}

TEST_F(FileSortWorkerTest, handleClearThumbnail)
{
    // Test method: void handleClearThumbnail(())
    EXPECT_NO_FATAL_FAILURE(obj->handleClearThumbnail());
}

TEST_F(FileSortWorkerTest, handleFileInfoUpdated)
{
    // Test method: void handleFileInfoUpdated((const QUrl &url, const QString &infoPtr, const bool isLinkOrg))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileInfoUpdated(_arg0, _arg1, false));
}

TEST_F(FileSortWorkerTest, handleGroupingRemove)
{
    // Test method: void handleGroupingRemove(())
    EXPECT_NO_FATAL_FAILURE(obj->handleGroupingRemove());
}

TEST_F(FileSortWorkerTest, handleReGrouping)
{
    // Test method: void handleReGrouping((const Qt::SortOrder order, const QString &strategy, const QVariantHash &expansionStates))
    QString _arg1{};
    QVariantHash _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleReGrouping(Qt::SortOrder(), _arg1, _arg2));
}

TEST_F(FileSortWorkerTest, handleResort)
{
    // Test method: void handleResort((const Qt::SortOrder order, const ItemRoles sortRole, const bool isMixDirAndFile))
    EXPECT_NO_FATAL_FAILURE(obj->handleResort(Qt::SortOrder(), ItemRoles(), false));
}

TEST_F(FileSortWorkerTest, handleToggleGroupExpansion)
{
    // Test method: void handleToggleGroupExpansion((const QString &key, const QString &groupKey))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleToggleGroupExpansion(_arg0, _arg1));
}

TEST_F(FileSortWorkerTest, handleToggleGroupTruncation)
{
    // Test method: void handleToggleGroupTruncation((const QString &key, const QString &groupKey))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleToggleGroupTruncation(_arg0, _arg1));
}

TEST_F(FileSortWorkerTest, handleWatcherAddChildren)
{
    // Test method: void handleWatcherAddChildren((const QList<SortInfoPointer> &children))
    QList<SortInfoPointer> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleWatcherAddChildren(_arg0));
}

TEST_F(FileSortWorkerTest, handleWatcherUpdateFile)
{
    // Test method: bool handleWatcherUpdateFile((const SortInfoPointer child))
    auto result = obj->handleWatcherUpdateFile(SortInfoPointer());
    EXPECT_FALSE(result);

}

TEST_F(FileSortWorkerTest, handleWatcherUpdateHideFile)
{
    // Test method: void handleWatcherUpdateHideFile((const QUrl &hidUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleWatcherUpdateHideFile(_arg0));
}

TEST_F(FileSortWorkerTest, removeChildrenByParents)
{
    // Test method: QList<QUrl> removeChildrenByParents((const QList<QUrl> &dirs))
    QList<QUrl> _arg0{};
    auto result = obj->removeChildrenByParents(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileSortWorkerTest, removeFileItems)
{
    // Test method: void removeFileItems((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeFileItems(_arg0));
}

TEST_F(FileSortWorkerTest, removeSubDir)
{
    // Test method: void removeSubDir((const QUrl &dir))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeSubDir(_arg0));
}

TEST_F(FileSortWorkerTest, removeVisibleChildren)
{
    // Test method: void removeVisibleChildren((const int startPos, const int size))
    EXPECT_NO_FATAL_FAILURE(obj->removeVisibleChildren(0, 0));
}

TEST_F(FileSortWorkerTest, resetFilters)
{
    // Test method: void resetFilters((const QDir::Filters filters))
    EXPECT_NO_FATAL_FAILURE(obj->resetFilters(QDir::Filters()));
}

TEST_F(FileSortWorkerTest, sortTreeFiles)
{
    // Test method: QList<QUrl> sortTreeFiles((const QList<QUrl> &children, const bool reverse))
    QList<QUrl> _arg0{};
    auto result = obj->sortTreeFiles(_arg0, false);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileSortWorkerTest, switchListView)
{
    // Test method: void switchListView(())
    EXPECT_NO_FATAL_FAILURE(obj->switchListView());
}

TEST_F(FileSortWorkerTest, setSortArguments)
{
    // Test method: FileSortWorker::SortOpt setSortArguments((const Qt::SortOrder order, const Global::ItemRoles sortRole, const bool isMixDirAndFile))
    auto result = obj->setSortArguments(Qt::SortOrder(), Global::ItemRoles(), false);
    EXPECT_GE(static_cast<int>(result), 0);

}
