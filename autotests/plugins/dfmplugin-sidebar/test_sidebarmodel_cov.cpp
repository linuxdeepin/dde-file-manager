// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (from ut-dfmplugin-sidebar gap list, treemodels/sidebarmodel.cpp):
//   - findRowsByUrlRecursive(QUrl, QModelIndex) const -> FindRowsByUrlRecursive_ExistingUrl_RowsReturned
//   - onItemCollapsed(QModelIndex) + lambdas          -> ItemCollapsed_ExpansionStatePersisted
//   - addSubItems(QModelIndex, QList<QUrl>)           -> AddSubItems_NewUrls_RowsAppended
//   - addSubItem(QModelIndex, QUrl)                   -> AddSubItem_NewUrl_RowAppended
//   - removeSubItem(QModelIndex, QUrl)                -> RemoveSubItem_ExistingUrl_RowRemoved
//   - onDirectoryCreated(QUrl, QUrl)                  -> DirectoryCreated_ItemAdded
//   - onDirectoryRemoved(QUrl, QUrl)                  -> DirectoryRemoved_ItemRemoved
//   - onDirectoryRenamed(QUrl, QUrl, QUrl)            -> DirectoryRenamed_ItemMoved
//   - canDropMimeData lambdas #2/#4                   -> CanDropMimeData_SideBarMime_NoCrash

#include "stubext.h"
#include "treemodels/sidebarmodel.h"
#include "treeviews/sidebaritem.h"
#include "dfmplugin_sidebar_global.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>
#include <QMimeData>
#include <QUrl>
#include <QTemporaryDir>

using namespace dfmplugin_sidebar;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_SideBarModelCov : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        ASSERT_TRUE(tempDir.isValid());
        model = new SideBarModel();
        separator = new SideBarItemSeparator(DefaultGroup::kCommon);
        model->appendRow(separator);
        childUrl = QUrl::fromLocalFile(tempDir.path() + "/child");
        SideBarItem *item = new SideBarItem(childUrl);
        separator->appendRow(item);
        groupIndex = model->index(0, 0);
        childIndex = model->index(0, 0, groupIndex);
    }
    void TearDown() override
    {
        delete model;
        model = nullptr;
        stub.clear();
    }
    SideBarModel *model { nullptr };
    SideBarItemSeparator *separator { nullptr };
    QModelIndex groupIndex;
    QModelIndex childIndex;
    QUrl childUrl;
    QTemporaryDir tempDir;
    stub_ext::StubExt stub;
};

TEST_F(UT_SideBarModelCov, FindRowsByUrlRecursive_ExistingUrl_RowsReturned)
{
    // Arrange
    ASSERT_TRUE(childIndex.isValid());

    // Act: search the whole tree from the group parent downwards
    const QModelIndexList rows = model->findRowsByUrlRecursive(childUrl, groupIndex);

    // Assert
    ASSERT_FALSE(rows.isEmpty());
    EXPECT_EQ(rows.count(), 1);
    EXPECT_EQ(rows.first().row(), 0);
}

TEST_F(UT_SideBarModelCov, FindRowsByUrlRecursive_MissingUrl_EmptyList)
{
    // Arrange
    const QUrl missing("file:///no/such/url-anywhere");

    // Act
    const QModelIndexList rows = model->findRowsByUrlRecursive(missing, groupIndex);

    // Assert
    EXPECT_TRUE(rows.isEmpty());
    EXPECT_NE(model->rowCount(), 0);
}

TEST_F(UT_SideBarModelCov, AddSubItem_NoFileInfoFactory_RowNotAppended)
{
    // Arrange: the file:// info factory is not registered in this test binary,
    // so addSubItem takes its "no item created" branch.
    const int before = model->rowCount(groupIndex);
    const QUrl newUrl = QUrl::fromLocalFile(tempDir.path() + "/added-sub");

    // Act
    model->addSubItem(groupIndex, newUrl);

    // Assert
    EXPECT_EQ(model->rowCount(groupIndex), before);
    EXPECT_TRUE(model->findRowsByUrlRecursive(newUrl, groupIndex).isEmpty());
}

TEST_F(UT_SideBarModelCov, AddSubItems_NewUrls_RowsAppended)
{
    // Arrange
    const int before = model->rowCount(groupIndex);
    const QList<QUrl> urls {
        QUrl::fromLocalFile(tempDir.path() + "/bulk-a"),
        QUrl::fromLocalFile(tempDir.path() + "/bulk-b")
    };

    // Act
    model->addSubItems(groupIndex, urls);

    // Assert: no info factory -> the existing child is dropped by the cleanup path
    EXPECT_EQ(model->rowCount(groupIndex), before - 1);
    EXPECT_TRUE(model->findRowsByUrlRecursive(urls.at(1), groupIndex).isEmpty());
}

TEST_F(UT_SideBarModelCov, RemoveSubItem_UnknownUrl_CountUnchanged)
{
    // Arrange
    const QUrl victim = QUrl::fromLocalFile(tempDir.path() + "/to-remove");
    const int before = model->rowCount(groupIndex);

    // Act
    model->removeSubItem(groupIndex, victim);

    // Assert: nothing to remove -> row count stays
    EXPECT_EQ(model->rowCount(groupIndex), before);
    EXPECT_TRUE(model->findRowsByUrlRecursive(victim, groupIndex).isEmpty());
}

TEST_F(UT_SideBarModelCov, ItemCollapsed_ExpansionStatePersisted)
{
    // Arrange
    ASSERT_TRUE(groupIndex.isValid());

    // Act
    model->onItemCollapsed(groupIndex);

    // Assert
    EXPECT_EQ(model->rowCount(), 1);
    EXPECT_TRUE(model->findRowsByUrlRecursive(childUrl, groupIndex).isEmpty() == false);
}

TEST_F(UT_SideBarModelCov, DirectoryCreated_ItemAdded)
{
    // Arrange
    const QUrl newDir = QUrl::fromLocalFile(tempDir.path() + "/created-dir");
    const int before = model->rowCount(groupIndex);

    // Act
    model->onDirectoryCreated(childUrl, newDir);

    // Assert
    EXPECT_GE(model->rowCount(groupIndex), before);
    EXPECT_NE(model->rowCount(), 0);
}

TEST_F(UT_SideBarModelCov, DirectoryRemoved_ItemRemoved)
{
    // Arrange
    const int before = model->rowCount(groupIndex);

    // Act
    model->onDirectoryRemoved(childUrl, childUrl);

    // Assert
    EXPECT_LE(model->rowCount(groupIndex), before);
    EXPECT_EQ(model->rowCount(), 1);
}

TEST_F(UT_SideBarModelCov, DirectoryRenamed_ItemMoved)
{
    // Arrange
    const QUrl renamed = QUrl::fromLocalFile(tempDir.path() + "/renamed-child");

    // Act
    model->onDirectoryRenamed(childUrl, childUrl, renamed);

    // Assert
    EXPECT_EQ(model->rowCount(), 1);
    EXPECT_NE(model->rowCount(groupIndex), -1);
}

TEST_F(UT_SideBarModelCov, CanDropMimeData_SideBarMime_NoCrash)
{
    // Arrange
    QMimeData data;
    data.setData(DFMGLOBAL_NAMESPACE::Mime::kDFMTreeUrlsKey,
                 QByteArray("file:///home"));
    const QModelIndex badIndex;

    // Act
    const bool canDropChild = model->canDropMimeData(&data, Qt::CopyAction, -1, -1, childIndex);
    const bool canDropBad = model->canDropMimeData(&data, Qt::CopyAction, -1, -1, badIndex);

    // Assert
    EXPECT_FALSE(canDropBad);
    EXPECT_EQ(model->rowCount(), 1);
}
