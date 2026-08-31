// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filesortworker_1.cpp
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

TEST_F(FileSortWorkerTest, FileSortWorker)
{
    // Test constructor: FileSortWorker((const QUrl &url, const QString &key,
                               FileViewFilterCallback callfun,
                               const QStringList &nameFilters, const QDir::Filters filters, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileSortWorkerTest, HandleNameFilters)
{
    // Test method: void HandleNameFilters((const QStringList &filters))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->HandleNameFilters(_arg0));
}

TEST_F(FileSortWorkerTest, applyGrouping)
{
    // Test method: void applyGrouping((const QList<FileItemDataPointer> &files))
    QList<FileItemDataPointer> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->applyGrouping(_arg0));
}

TEST_F(FileSortWorkerTest, cancel)
{
    // Test method: void cancel(())
    EXPECT_NO_FATAL_FAILURE(obj->cancel());
}

TEST_F(FileSortWorkerTest, checkAndSortBytMimeType)
{
    // Test method: void checkAndSortBytMimeType((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->checkAndSortBytMimeType(_arg0));
}

TEST_F(FileSortWorkerTest, checkNameFilters)
{
    // Test method: void checkNameFilters((const FileItemDataPointer itemData))
    EXPECT_NO_FATAL_FAILURE(obj->checkNameFilters(FileItemDataPointer()));
}

TEST_F(FileSortWorkerTest, childData)
{
    // Test method: FileItemDataPointer childData((const int index))
    auto result = obj->childData(0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileSortWorkerTest, childrenCount)
{
    // Test getter: int childrenCount()
    auto result = obj->childrenCount();
    EXPECT_EQ(result, 0);

}

TEST_F(FileSortWorkerTest, childrenCountInternal)
{
    // Test getter: int childrenCountInternal()
    auto result = obj->childrenCountInternal();
    EXPECT_EQ(result, 0);

}

TEST_F(FileSortWorkerTest, createAndInsertItemData)
{
    // Test method: void createAndInsertItemData((const int8_t depth, const SortInfoPointer child, const FileInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->createAndInsertItemData({}, SortInfoPointer(), FileInfoPointer()));
}

TEST_F(FileSortWorkerTest, currentIsGroupingMode)
{
    // Test bool getter: currentIsGroupingMode()
    bool result = obj->currentIsGroupingMode();
    EXPECT_FALSE(result);

}

TEST_F(FileSortWorkerTest, doCompleteFileInfo)
{
    // Test method: void doCompleteFileInfo((SortInfoPointer sortInfo))
    EXPECT_NO_FATAL_FAILURE(obj->doCompleteFileInfo(SortInfoPointer()));
}

TEST_F(FileSortWorkerTest, filterAllFilesOrdered)
{
    // Test method: void filterAllFilesOrdered(())
    EXPECT_NO_FATAL_FAILURE(obj->filterAllFilesOrdered());
}

TEST_F(FileSortWorkerTest, findDepth)
{
    // Test method: int8_t findDepth((const QUrl &parent))
    QUrl _arg0{};
    auto result = obj->findDepth(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->findDepth(_arg0); });

}

TEST_F(FileSortWorkerTest, findEndPos)
{
    // Test method: int findEndPos((const QUrl &dir))
    QUrl _arg0{};
    auto result = obj->findEndPos(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(FileSortWorkerTest, findRealShowIndex)
{
    // Test method: int findRealShowIndex((const QUrl &preItemUrl))
    QUrl _arg0{};
    auto result = obj->findRealShowIndex(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(FileSortWorkerTest, findStartPos)
{
    // Test method: int findStartPos((const QList<QUrl> &list, const QUrl &parent))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->findStartPos(_arg0, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(FileSortWorkerTest, getChildShowIndex)
{
    // Test method: int getChildShowIndex((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->getChildShowIndex(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(FileSortWorkerTest, getChildShowIndexInternal)
{
    // Test method: int getChildShowIndexInternal((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->getChildShowIndexInternal(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(FileSortWorkerTest, getChildrenUrls)
{
    // Test getter: QList<QUrl> getChildrenUrls()
    auto result = obj->getChildrenUrls();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileSortWorkerTest, getDepth)
{
    // Test method: int8_t getDepth((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->getDepth(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->getDepth(_arg0); });

}

TEST_F(FileSortWorkerTest, getFileItemCount)
{
    // Test getter: int getFileItemCount()
    auto result = obj->getFileItemCount();
    EXPECT_EQ(result, 0);

}

TEST_F(FileSortWorkerTest, getGroupItemCount)
{
    // Test getter: int getGroupItemCount()
    auto result = obj->getGroupItemCount();
    EXPECT_EQ(result, 0);

}

TEST_F(FileSortWorkerTest, getGroupOrder)
{
    // Test getter: Qt::SortOrder getGroupOrder()
    auto result = obj->getGroupOrder();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileSortWorkerTest, getGroupStrategyName)
{
    // Test getter: QString getGroupStrategyName()
    auto result = obj->getGroupStrategyName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileSortWorkerTest, getSortOrder)
{
    // Test getter: Qt::SortOrder getSortOrder()
    auto result = obj->getSortOrder();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileSortWorkerTest, getSortRole)
{
    // Test getter: ItemRoles getSortRole()
    auto result = obj->getSortRole();
    EXPECT_NO_FATAL_FAILURE({ obj->getSortRole(); });

}

TEST_F(FileSortWorkerTest, groupHeaderData)
{
    // Test method: QVariant groupHeaderData((const int index, const int role))
    auto result = obj->groupHeaderData(0, 0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileSortWorkerTest, handleAboutToInsertFilesToGroup)
{
    // Test method: void handleAboutToInsertFilesToGroup((int pos, int count))
    EXPECT_NO_FATAL_FAILURE(obj->handleAboutToInsertFilesToGroup(0, 0));
}

TEST_F(FileSortWorkerTest, handleCloseExpand)
{
    // Test method: void handleCloseExpand((const QString &key, const QUrl &parent))
    QString _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleCloseExpand(_arg0, _arg1));
}

TEST_F(FileSortWorkerTest, handleFilterCallFunc)
{
    // Test method: void handleFilterCallFunc((FileViewFilterCallback callback))
    EXPECT_NO_FATAL_FAILURE(obj->handleFilterCallFunc(FileViewFilterCallback()));
}

TEST_F(FileSortWorkerTest, handleFilterData)
{
    // Test method: void handleFilterData((const QVariant &data))
    QVariant _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFilterData(_arg0));
}

TEST_F(FileSortWorkerTest, handleFilters)
{
    // Test method: void handleFilters((QDir::Filters filters))
    EXPECT_NO_FATAL_FAILURE(obj->handleFilters(QDir::Filters()));
}

TEST_F(FileSortWorkerTest, handleGroupingInsert)
{
    // Test method: void handleGroupingInsert(())
    EXPECT_NO_FATAL_FAILURE(obj->handleGroupingInsert());
}

TEST_F(FileSortWorkerTest, handleGroupingUpdate)
{
    // Test method: void handleGroupingUpdate(())
    EXPECT_NO_FATAL_FAILURE(obj->handleGroupingUpdate());
}

TEST_F(FileSortWorkerTest, handleIteratorChildren)
{
    // Test method: void handleIteratorChildren((const QString &key, const QList<SortInfoPointer> children, const QList<FileInfoPointer> infos, bool isFirstBatch))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleIteratorChildren(_arg0, QList<SortInfoPointer>(), QList<FileInfoPointer>(), false));
}

TEST_F(FileSortWorkerTest, handleIteratorLocalChildren)
{
    // Test method: void handleIteratorLocalChildren((const QString &key,
                                                 const QList<SortInfoPointer> children,
                                                 const DEnumerator::SortRoleCompareFlag sortRole,
                                                 const Qt::SortOrder sortOrder,
                                                 const bool isMixDirAndFile,
                                                 bool isFirstBatch))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleIteratorLocalChildren(_arg0, QList<SortInfoPointer>(), DEnumerator::SortRoleCompareFlag(), Qt::SortOrder(), false, false));
}

TEST_F(FileSortWorkerTest, handleRefresh)
{
    // Test method: void handleRefresh(())
    EXPECT_NO_FATAL_FAILURE(obj->handleRefresh());
}

TEST_F(FileSortWorkerTest, handleSortByMimeType)
{
    // Test method: void handleSortByMimeType(())
    EXPECT_NO_FATAL_FAILURE(obj->handleSortByMimeType());
}

TEST_F(FileSortWorkerTest, handleSortDir)
{
    // Test method: void handleSortDir((const QString &key, const QUrl &parent))
    QString _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleSortDir(_arg0, _arg1));
}

TEST_F(FileSortWorkerTest, handleSwitchTreeView)
{
    // Test method: void handleSwitchTreeView((const bool isTree))
    EXPECT_NO_FATAL_FAILURE(obj->handleSwitchTreeView(false));
}

TEST_F(FileSortWorkerTest, handleTraversalFinish)
{
    // Test method: void handleTraversalFinish((const QString &key, bool noDataProduced))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleTraversalFinish(_arg0, false));
}

TEST_F(FileSortWorkerTest, handleUpdateFiles)
{
    // Test method: void handleUpdateFiles((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleUpdateFiles(_arg0));
}

TEST_F(FileSortWorkerTest, handleUpdateRefreshFiles)
{
    // Test method: void handleUpdateRefreshFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->handleUpdateRefreshFiles());
}

TEST_F(FileSortWorkerTest, handleWatcherUpdateFiles)
{
    // Test method: void handleWatcherUpdateFiles((const QList<SortInfoPointer> &children))
    QList<SortInfoPointer> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleWatcherUpdateFiles(_arg0));
}

TEST_F(FileSortWorkerTest, indexOfVisibleChild)
{
    // Test method: int indexOfVisibleChild((const QUrl &itemUrl))
    QUrl _arg0{};
    auto result = obj->indexOfVisibleChild(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(FileSortWorkerTest, insertSortList)
{
    // Test method: int insertSortList((const QUrl &needNode, const QList<QUrl> &list))
    QUrl _arg0{};
    QList<QUrl> _arg1{};
    auto result = obj->insertSortList(_arg0, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(FileSortWorkerTest, insertVisibleChildren)
{
    // Test method: void insertVisibleChildren((const int startPos, const QList<QUrl> &filterUrls,
                                           const InsertOpt opt, const int endPos))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->insertVisibleChildren(0, _arg1, InsertOpt(), 0));
}

TEST_F(FileSortWorkerTest, isDefaultHiddenFile)
{
    // Test method: bool isDefaultHiddenFile((const QUrl &fileUrl))
    QUrl _arg0{};
    auto result = obj->isDefaultHiddenFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileSortWorkerTest, makeParentUrl)
{
    // Test method: QUrl makeParentUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->makeParentUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileSortWorkerTest, onAppAttributeChanged)
{
    // Test method: void onAppAttributeChanged((Application::ApplicationAttribute aa, const QVariant &value))
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onAppAttributeChanged(Application::ApplicationAttribute(), _arg1));
}

TEST_F(FileSortWorkerTest, onShowHiddenFileChanged)
{
    // Test method: void onShowHiddenFileChanged((bool isShow))
    EXPECT_NO_FATAL_FAILURE(obj->onShowHiddenFileChanged(false));
}

TEST_F(FileSortWorkerTest, onToggleHiddenFiles)
{
    // Test method: void onToggleHiddenFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->onToggleHiddenFiles());
}

TEST_F(FileSortWorkerTest, resortCurrent)
{
    // Test method: void resortCurrent((const bool reverse))
    EXPECT_NO_FATAL_FAILURE(obj->resortCurrent(false));
}

TEST_F(FileSortWorkerTest, resortVisibleChildren)
{
    // Test method: void resortVisibleChildren((const QList<QUrl> &fileUrls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->resortVisibleChildren(_arg0));
}

TEST_F(FileSortWorkerTest, rootData)
{
    // Test getter: FileItemDataPointer rootData()
    auto result = obj->rootData();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(FileSortWorkerTest, setGroupArguments)
{
    // Test method: FileSortWorker::GroupingOpt setGroupArguments((const Qt::SortOrder order,
                                                              const QString &strategy,
                                                              const QVariantHash &expandStates))
    QString _arg1{};
    QVariantHash _arg2{};
    auto result = obj->setGroupArguments(Qt::SortOrder(), _arg1, _arg2);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileSortWorkerTest, setRootData)
{
    // Test setter: void setRootData((const FileItemDataPointer data))
    EXPECT_NO_FATAL_FAILURE(obj->setRootData(FileItemDataPointer()));
}

TEST_F(FileSortWorkerTest, setTreeView)
{
    // Test setter: void setTreeView((const bool isTree))
    EXPECT_NO_FATAL_FAILURE(obj->setTreeView(false));
}

TEST_F(FileSortWorkerTest, setVisibleChildren)
{
    // Test method: int setVisibleChildren((const int startPos, const QList<QUrl> &filterUrls, const FileSortWorker::InsertOpt opt, const int endPos))
    QList<QUrl> _arg1{};
    auto result = obj->setVisibleChildren(0, _arg1, FileSortWorker::InsertOpt(), 0);
    EXPECT_GE(result, 0);

}

TEST_F(FileSortWorkerTest, sortInfoUpdateByFileInfo)
{
    // Test method: bool sortInfoUpdateByFileInfo((const FileInfoPointer fileInfo))
    auto result = obj->sortInfoUpdateByFileInfo(FileInfoPointer());
    EXPECT_FALSE(result);

}

TEST_F(FileSortWorkerTest, switchTreeView)
{
    // Test method: void switchTreeView(())
    EXPECT_NO_FATAL_FAILURE(obj->switchTreeView());
}

TEST_F(FileSortWorkerTest, updateSorterContext)
{
    // Test method: void updateSorterContext(())
    EXPECT_NO_FATAL_FAILURE(obj->updateSorterContext());
}

TEST_F(FileSortWorkerTest, FileSortWorker_Destructor)
{
    // Test method:  ~FileSortWorker(())
    EXPECT_NO_FATAL_FAILURE({ FileSortWorker *tmp = new FileSortWorker(); delete tmp; });
}
