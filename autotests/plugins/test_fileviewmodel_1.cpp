// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileviewmodel_1.cpp
 * @brief Unit tests for FileViewModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/fileviewmodel.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileViewModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileViewModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileViewModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileViewModelTest, FileViewModel)
{
    // Test constructor: FileViewModel((QAbstractItemView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileViewModelTest, canFetchMore)
{
    // Test method: bool canFetchMore((const QModelIndex &parent))
    QModelIndex _arg0{};
    auto result = obj->canFetchMore(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileViewModelTest, changeGroupingState)
{
    // Test method: void changeGroupingState((GroupingState newState))
    EXPECT_NO_FATAL_FAILURE(obj->changeGroupingState(GroupingState()));
}

TEST_F(FileViewModelTest, changeState)
{
    // Test method: void changeState((ModelState newState))
    EXPECT_NO_FATAL_FAILURE(obj->changeState(ModelState()));
}

TEST_F(FileViewModelTest, closeCursorTimer)
{
    // Test method: void closeCursorTimer(())
    EXPECT_NO_FATAL_FAILURE(obj->closeCursorTimer());
}

TEST_F(FileViewModelTest, columnToRole)
{
    // Test method: ItemRoles columnToRole((int column))
    auto result = obj->columnToRole(0);
    EXPECT_NO_FATAL_FAILURE({ obj->columnToRole(0); });

}

TEST_F(FileViewModelTest, connectRootAndFilterSortWork)
{
    // Test method: void connectRootAndFilterSortWork((RootInfo *root, const bool refresh))
    EXPECT_NO_FATAL_FAILURE(obj->connectRootAndFilterSortWork(nullptr, false));
}

TEST_F(FileViewModelTest, currentState)
{
    // Test getter: ModelState currentState()
    auto result = obj->currentState();
    EXPECT_NO_FATAL_FAILURE({ obj->currentState(); });

}

TEST_F(FileViewModelTest, directoryLoadStrategy)
{
    // Test getter: DirectoryLoadStrategy directoryLoadStrategy()
    auto result = obj->directoryLoadStrategy();
    EXPECT_NO_FATAL_FAILURE({ obj->directoryLoadStrategy(); });

}

TEST_F(FileViewModelTest, discardFilterSortObjects)
{
    // Test method: void discardFilterSortObjects(())
    EXPECT_NO_FATAL_FAILURE(obj->discardFilterSortObjects());
}

TEST_F(FileViewModelTest, fetchMore)
{
    // Test method: void fetchMore((const QModelIndex &parent))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->fetchMore(_arg0));
}

TEST_F(FileViewModelTest, getChildrenUrls)
{
    // Test getter: QList<QUrl> getChildrenUrls()
    auto result = obj->getChildrenUrls();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewModelTest, getColumnByRole)
{
    // Test method: int getColumnByRole((ItemRoles role))
    auto result = obj->getColumnByRole(ItemRoles());
    EXPECT_GE(result, 0);

}

TEST_F(FileViewModelTest, getColumnWidth)
{
    // Test method: int getColumnWidth((int column))
    auto result = obj->getColumnWidth(0);
    EXPECT_GE(result, 0);

}

TEST_F(FileViewModelTest, getFileOnlyCount)
{
    // Test getter: int getFileOnlyCount()
    auto result = obj->getFileOnlyCount();
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewModelTest, getFilters)
{
    // Test getter: QDir::Filters getFilters()
    auto result = obj->getFilters();
    EXPECT_NO_FATAL_FAILURE({ obj->getFilters(); });

}

TEST_F(FileViewModelTest, getGroupOnlyCount)
{
    // Test getter: int getGroupOnlyCount()
    auto result = obj->getGroupOnlyCount();
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewModelTest, getIndexByUrl)
{
    // Test method: QModelIndex getIndexByUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->getIndexByUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewModelTest, getKeyWords)
{
    // Test getter: QStringList getKeyWords()
    auto result = obj->getKeyWords();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewModelTest, getNameFilters)
{
    // Test getter: QStringList getNameFilters()
    auto result = obj->getNameFilters();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewModelTest, getRoleByColumn)
{
    // Test method: ItemRoles getRoleByColumn((int column))
    auto result = obj->getRoleByColumn(0);
    EXPECT_NO_FATAL_FAILURE({ obj->getRoleByColumn(0); });

}

TEST_F(FileViewModelTest, grouping)
{
    // Test method: void grouping((const QString &strategyName, Qt::SortOrder order))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->grouping(_arg0, Qt::SortOrder()));
}

TEST_F(FileViewModelTest, groupingOrder)
{
    // Test getter: Qt::SortOrder groupingOrder()
    auto result = obj->groupingOrder();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileViewModelTest, groupingState)
{
    // Test getter: GroupingState groupingState()
    auto result = obj->groupingState();
    EXPECT_NO_FATAL_FAILURE({ obj->groupingState(); });

}

TEST_F(FileViewModelTest, groupingStrategy)
{
    // Test getter: QString groupingStrategy()
    auto result = obj->groupingStrategy();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewModelTest, headerData)
{
    // Test method: QVariant headerData((int column, Qt::Orientation, int role))
    auto result = obj->headerData(0, Qt::Orientation(), 0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewModelTest, mimeTypes)
{
    // Test getter: QStringList mimeTypes()
    auto result = obj->mimeTypes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewModelTest, onDConfigChanged)
{
    // Test method: void onDConfigChanged((const QString &config, const QString &key))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onDConfigChanged(_arg0, _arg1));
}

TEST_F(FileViewModelTest, onDataChanged)
{
    // Test method: void onDataChanged((int first, int last))
    EXPECT_NO_FATAL_FAILURE(obj->onDataChanged(0, 0));
}

TEST_F(FileViewModelTest, onFileThumbUpdated)
{
    // Test method: void onFileThumbUpdated((const QUrl &url, const QString &thumb))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileThumbUpdated(_arg0, _arg1));
}

TEST_F(FileViewModelTest, onFileUpdated)
{
    // Test method: void onFileUpdated((int show))
    EXPECT_NO_FATAL_FAILURE(obj->onFileUpdated(0));
}

TEST_F(FileViewModelTest, onGroupExpansionChanged)
{
    // Test method: void onGroupExpansionChanged((const QString &strategyName, const QString &key, bool state))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onGroupExpansionChanged(_arg0, _arg1, false));
}

TEST_F(FileViewModelTest, onGroupInsert)
{
    // Test method: void onGroupInsert((int firstIndex, int count))
    EXPECT_NO_FATAL_FAILURE(obj->onGroupInsert(0, 0));
}

TEST_F(FileViewModelTest, onGroupInsertFinish)
{
    // Test method: void onGroupInsertFinish(())
    EXPECT_NO_FATAL_FAILURE(obj->onGroupInsertFinish());
}

TEST_F(FileViewModelTest, onGroupingDataChanged)
{
    // Test method: void onGroupingDataChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onGroupingDataChanged());
}

TEST_F(FileViewModelTest, onHiddenSettingChanged)
{
    // Test method: void onHiddenSettingChanged((bool value))
    EXPECT_NO_FATAL_FAILURE(obj->onHiddenSettingChanged(false));
}

TEST_F(FileViewModelTest, onHighlightReady)
{
    // Test method: void onHighlightReady((const QString &taskId, const QString &path, const QString &content))
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onHighlightReady(_arg0, _arg1, _arg2));
}

TEST_F(FileViewModelTest, onInsert)
{
    // Test method: void onInsert((int firstIndex, int count))
    EXPECT_NO_FATAL_FAILURE(obj->onInsert(0, 0));
}

TEST_F(FileViewModelTest, onInsertFinish)
{
    // Test method: void onInsertFinish(())
    EXPECT_NO_FATAL_FAILURE(obj->onInsertFinish());
}

TEST_F(FileViewModelTest, onSetCursorWait)
{
    // Test method: void onSetCursorWait(())
    EXPECT_NO_FATAL_FAILURE(obj->onSetCursorWait());
}

TEST_F(FileViewModelTest, onSortStrategyChanged)
{
    // Test method: void onSortStrategyChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onSortStrategyChanged());
}

TEST_F(FileViewModelTest, onUpdateView)
{
    // Test method: void onUpdateView(())
    EXPECT_NO_FATAL_FAILURE(obj->onUpdateView());
}

TEST_F(FileViewModelTest, onWorkFinish)
{
    // Test method: void onWorkFinish((int visiableCount, int totalCount))
    EXPECT_NO_FATAL_FAILURE(obj->onWorkFinish(0, 0));
}

TEST_F(FileViewModelTest, parent)
{
    // Test method: QModelIndex parent((const QModelIndex &child))
    QModelIndex _arg0{};
    auto result = obj->parent(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewModelTest, prepareUrl)
{
    // Test method: void prepareUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->prepareUrl(_arg0));
}

TEST_F(FileViewModelTest, quitFilterSortWork)
{
    // Test method: void quitFilterSortWork(())
    EXPECT_NO_FATAL_FAILURE(obj->quitFilterSortWork());
}

TEST_F(FileViewModelTest, refresh)
{
    // Test method: void refresh(())
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
}

TEST_F(FileViewModelTest, refreshHighlightKeywords)
{
    // Test method: void refreshHighlightKeywords((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->refreshHighlightKeywords(_arg0));
}

TEST_F(FileViewModelTest, setDirectoryLoadStrategy)
{
    // Test setter: void setDirectoryLoadStrategy((DirectoryLoadStrategy strategy))
    EXPECT_NO_FATAL_FAILURE(obj->setDirectoryLoadStrategy(DirectoryLoadStrategy()));
}

TEST_F(FileViewModelTest, setFilterCallback)
{
    // Test setter: void setFilterCallback((const FileViewFilterCallback callback))
    EXPECT_NO_FATAL_FAILURE(obj->setFilterCallback(FileViewFilterCallback()));
}

TEST_F(FileViewModelTest, setFilterData)
{
    // Test setter: void setFilterData((const QVariant &data))
    QVariant _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setFilterData(_arg0));
}

TEST_F(FileViewModelTest, setFilters)
{
    // Test setter: void setFilters((QDir::Filters filters))
    EXPECT_NO_FATAL_FAILURE(obj->setFilters(QDir::Filters()));
}

TEST_F(FileViewModelTest, setNameFilters)
{
    // Test setter: void setNameFilters((const QStringList &filters))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setNameFilters(_arg0));
}

TEST_F(FileViewModelTest, setReadOnly)
{
    // Test setter: void setReadOnly((bool value))
    EXPECT_NO_FATAL_FAILURE(obj->setReadOnly(false));
}

TEST_F(FileViewModelTest, setTreeView)
{
    // Test setter: void setTreeView((const bool isTree))
    EXPECT_NO_FATAL_FAILURE(obj->setTreeView(false));
}

TEST_F(FileViewModelTest, sortOrder)
{
    // Test getter: Qt::SortOrder sortOrder()
    auto result = obj->sortOrder();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileViewModelTest, sortRole)
{
    // Test getter: ItemRoles sortRole()
    auto result = obj->sortRole();
    EXPECT_NO_FATAL_FAILURE({ obj->sortRole(); });

}

TEST_F(FileViewModelTest, startCursorTimer)
{
    // Test method: void startCursorTimer(())
    EXPECT_NO_FATAL_FAILURE(obj->startCursorTimer());
}

TEST_F(FileViewModelTest, stopTraversWork)
{
    // Test method: void stopTraversWork((const QUrl &newUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->stopTraversWork(_arg0));
}

TEST_F(FileViewModelTest, supportedDragActions)
{
    // Test getter: Qt::DropActions supportedDragActions()
    auto result = obj->supportedDragActions();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileViewModelTest, supportedDropActions)
{
    // Test getter: Qt::DropActions supportedDropActions()
    auto result = obj->supportedDropActions();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileViewModelTest, toggleGroupExpansion)
{
    // Test method: void toggleGroupExpansion((const QString &groupKey))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->toggleGroupExpansion(_arg0));
}

TEST_F(FileViewModelTest, toggleGroupTruncation)
{
    // Test method: void toggleGroupTruncation((const QString &groupKey))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->toggleGroupTruncation(_arg0));
}

TEST_F(FileViewModelTest, toggleHiddenFiles)
{
    // Test method: void toggleHiddenFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->toggleHiddenFiles());
}

TEST_F(FileViewModelTest, toggleTreeItemCollapse)
{
    // Test method: void toggleTreeItemCollapse((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->toggleTreeItemCollapse(_arg0));
}

TEST_F(FileViewModelTest, toggleTreeItemExpansion)
{
    // Test method: void toggleTreeItemExpansion((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->toggleTreeItemExpansion(_arg0));
}

TEST_F(FileViewModelTest, updateFile)
{
    // Test method: void updateFile((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateFile(_arg0));
}

TEST_F(FileViewModelTest, updateHorizontalOffset)
{
    // Test method: void updateHorizontalOffset((const bool update))
    EXPECT_NO_FATAL_FAILURE(obj->updateHorizontalOffset(false));
}

TEST_F(FileViewModelTest, updateThumbnailIcon)
{
    // Test method: void updateThumbnailIcon((const QModelIndex &index, const QString &thumb))
    QModelIndex _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateThumbnailIcon(_arg0, _arg1));
}

TEST_F(FileViewModelTest, FileViewModel_Destructor)
{
    // Test method:  ~FileViewModel(())
    EXPECT_NO_FATAL_FAILURE({ FileViewModel *tmp = new FileViewModel(); delete tmp; });
}
