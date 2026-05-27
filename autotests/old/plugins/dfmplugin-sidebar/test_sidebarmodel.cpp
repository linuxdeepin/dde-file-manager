// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "treemodels/sidebarmodel.h"
#include "treeviews/sidebaritem.h"
#include "dfmplugin_sidebar_global.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-framework/event/event.h>

#include <QMimeData>
#include <QUrl>

using namespace dfmplugin_sidebar;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_SideBarModel : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        model = new SideBarModel();
        ASSERT_NE(model, nullptr);
    }

    virtual void TearDown() override
    {
        delete model;
        model = nullptr;
        stub.clear();
    }

protected:
    SideBarModel *model { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_SideBarModel, Constructor)
{
    EXPECT_NE(model, nullptr);
    EXPECT_EQ(model->rowCount(), 0);
}

TEST_F(UT_SideBarModel, ItemFromIndex_ValidIndex)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/test");
    SideBarItemSeparator *separator = new SideBarItemSeparator(DefaultGroup::kCommon);
    model->appendRow(separator);

    SideBarItem *item = new SideBarItem(testUrl);
    separator->appendRow(item);

    QModelIndex index = model->index(0, 0);
    SideBarItem *retrievedItem = model->itemFromIndex(index);

    EXPECT_NE(retrievedItem, nullptr);
    EXPECT_EQ(retrievedItem, separator);
}

TEST_F(UT_SideBarModel, ItemFromIndex_ByRow)
{
    SideBarItemSeparator *separator = new SideBarItemSeparator(DefaultGroup::kCommon);
    model->appendRow(separator);

    SideBarItem *retrievedItem = model->itemFromIndex(0);

    EXPECT_NE(retrievedItem, nullptr);
    EXPECT_EQ(retrievedItem, separator);
}

TEST_F(UT_SideBarModel, GroupItems)
{
    SideBarItemSeparator *separator1 = new SideBarItemSeparator(DefaultGroup::kCommon);
    SideBarItemSeparator *separator2 = new SideBarItemSeparator(DefaultGroup::kDevice);

    model->appendRow(separator1);
    model->appendRow(separator2);

    QList<SideBarItemSeparator *> groups = model->groupItems();

    EXPECT_EQ(groups.size(), 2);
    EXPECT_TRUE(groups.contains(separator1));
    EXPECT_TRUE(groups.contains(separator2));
}

TEST_F(UT_SideBarModel, SubItems_All)
{
    SideBarItemSeparator *separator = new SideBarItemSeparator(DefaultGroup::kCommon);
    model->appendRow(separator);

    QUrl url1 = QUrl::fromLocalFile("/home/test1");
    QUrl url2 = QUrl::fromLocalFile("/home/test2");

    SideBarItem *item1 = new SideBarItem(url1);
    SideBarItem *item2 = new SideBarItem(url2);
    item1->setGroup(DefaultGroup::kCommon);
    item2->setGroup(DefaultGroup::kCommon);

    separator->appendRow(item1);
    separator->appendRow(item2);

    QList<SideBarItem *> items = model->subItems();

    EXPECT_EQ(items.size(), 2);
}

TEST_F(UT_SideBarModel, SubItems_ByGroup)
{
    SideBarItemSeparator *separatorCommon = new SideBarItemSeparator(DefaultGroup::kCommon);
    SideBarItemSeparator *separatorDevice = new SideBarItemSeparator(DefaultGroup::kDevice);

    model->appendRow(separatorCommon);
    model->appendRow(separatorDevice);

    QUrl url1 = QUrl::fromLocalFile("/home/common1");
    QUrl url2 = QUrl::fromLocalFile("/dev/device1");

    SideBarItem *item1 = new SideBarItem(url1);
    SideBarItem *item2 = new SideBarItem(url2);
    item1->setGroup(DefaultGroup::kCommon);
    item2->setGroup(DefaultGroup::kDevice);

    separatorCommon->appendRow(item1);
    separatorDevice->appendRow(item2);

    QList<SideBarItem *> commonItems = model->subItems(DefaultGroup::kCommon);

    EXPECT_EQ(commonItems.size(), 1);
    EXPECT_EQ(commonItems.first(), item1);
}

TEST_F(UT_SideBarModel, InsertRow_NullItem)
{
    bool result = model->insertRow(0, nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_SideBarModel, InsertRow_InvalidRowIndex)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/invalid");
    SideBarItem *item = new SideBarItem(testUrl);
    item->setGroup(DefaultGroup::kCommon);

    bool result = model->insertRow(-5, item);
    EXPECT_FALSE(result);

    delete item;
}

TEST_F(UT_SideBarModel, InsertRow_Separator)
{
    SideBarItemSeparator *separator = new SideBarItemSeparator(DefaultGroup::kCommon);

    stub.set_lamda(&SideBarModel::findRowByUrl, [](const SideBarModel *, const QUrl &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return QModelIndex();   // Not found
    });

    bool result = model->insertRow(0, separator);
    EXPECT_TRUE(result);
}

TEST_F(UT_SideBarModel, InsertRow_SubItem)
{
    SideBarItemSeparator *separator = new SideBarItemSeparator(DefaultGroup::kCommon);
    model->appendRow(separator);

    QUrl testUrl = QUrl::fromLocalFile("/home/subitem");
    SideBarItem *item = new SideBarItem(testUrl);
    item->setGroup(DefaultGroup::kCommon);

    stub.set_lamda(&SideBarModel::findRowByUrl, [](const SideBarModel *, const QUrl &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return QModelIndex();   // Not found
    });

    bool result = model->insertRow(0, item);

    EXPECT_TRUE(result);
}

TEST_F(UT_SideBarModel, AppendRow_NullItem)
{
    int result = model->appendRow(nullptr);
    EXPECT_EQ(result, -1);
}

TEST_F(UT_SideBarModel, AppendRow_AlreadyExists)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/exists");
    SideBarItem *item = new SideBarItem(testUrl);

    stub.set_lamda(&SideBarModel::findRowByUrl, [](const SideBarModel *, const QUrl &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        QModelIndex index;
        index.r = 5;
        index.c = 0;
        // Simulate found at row 5
        return index;
    });

    int result = model->appendRow(item);
    EXPECT_EQ(result, 5);

    delete item;
}

TEST_F(UT_SideBarModel, AppendRow_Separator)
{
    SideBarItemSeparator *separator = new SideBarItemSeparator(DefaultGroup::kCommon);

    stub.set_lamda(&SideBarModel::findRowByUrl, [](const SideBarModel *, const QUrl &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        QModelIndex index;
        return index;   // Not found, row() returns -1
    });

    int result = model->appendRow(separator);

    EXPECT_GE(result, 0);
    EXPECT_EQ(model->rowCount(), 1);
}

TEST_F(UT_SideBarModel, AppendRow_SubItem_WithGroup)
{
    SideBarItemSeparator *separator = new SideBarItemSeparator(DefaultGroup::kCommon);
    model->appendRow(separator);

    QUrl testUrl = QUrl::fromLocalFile("/home/append");
    SideBarItem *item = new SideBarItem(testUrl);
    item->setGroup(DefaultGroup::kCommon);

    stub.set_lamda(&SideBarModel::findRowByUrl, [](const SideBarModel *, const QUrl &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return QModelIndex();   // Not found
    });

    int result = model->appendRow(item, true);

    EXPECT_GE(result, 0);
}

TEST_F(UT_SideBarModel, RemoveRow_InvalidUrl)
{
    QUrl invalidUrl;
    bool result = model->removeRow(invalidUrl);
    EXPECT_FALSE(result);
}

TEST_F(UT_SideBarModel, RemoveRow_NotFound)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/notfound");

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = model->removeRow(testUrl);
    EXPECT_FALSE(result);
}

TEST_F(UT_SideBarModel, RemoveRow_Success)
{
    SideBarItemSeparator *separator = new SideBarItemSeparator(DefaultGroup::kCommon);
    model->appendRow(separator);

    QUrl testUrl = QUrl::fromLocalFile("/home/remove");
    SideBarItem *item = new SideBarItem(testUrl);
    item->setGroup(DefaultGroup::kCommon);
    separator->appendRow(item);

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = model->removeRow(testUrl);
    EXPECT_TRUE(result);
}

TEST_F(UT_SideBarModel, UpdateRow_InvalidUrl)
{
    QUrl invalidUrl;
    ItemInfo info;

    // Should return early without crashing
    model->updateRow(invalidUrl, info);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarModel, UpdateRow_Success)
{
    SideBarItemSeparator *separator = new SideBarItemSeparator(DefaultGroup::kCommon);
    model->appendRow(separator);

    QUrl testUrl = QUrl::fromLocalFile("/home/update");
    SideBarItem *item = new SideBarItem(testUrl);
    item->setGroup(DefaultGroup::kCommon);
    separator->appendRow(item);

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&SideBarItem::itemInfo, []() -> ItemInfo {
        __DBG_STUB_INVOKE__
        ItemInfo info;
        info.findMeCb = nullptr;
        return info;
    });

    ItemInfo newInfo;
    newInfo.url = testUrl;
    newInfo.displayName = "Updated Name";
    newInfo.icon = QIcon::fromTheme("folder");
    newInfo.flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    newInfo.group = DefaultGroup::kCommon;
    newInfo.isEditable = true;

    model->updateRow(testUrl, newInfo);

    // Test passes if no crash
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarModel, FindRowByUrl_NotFound)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/notexist");

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    QModelIndex index = model->findRowByUrl(testUrl);
    EXPECT_FALSE(index.isValid());
}

TEST_F(UT_SideBarModel, FindRowByUrl_Success)
{
    SideBarItemSeparator *separator = new SideBarItemSeparator(DefaultGroup::kCommon);
    model->appendRow(separator);

    QUrl testUrl = QUrl::fromLocalFile("/home/find");
    SideBarItem *item = new SideBarItem(testUrl);
    item->setGroup(DefaultGroup::kCommon);
    separator->appendRow(item);

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QModelIndex index = model->findRowByUrl(testUrl);
    EXPECT_TRUE(index.isValid());
}

TEST_F(UT_SideBarModel, AddEmptyItem)
{
    int initialCount = model->rowCount();

    model->addEmptyItem();

    EXPECT_EQ(model->rowCount(), initialCount + 1);

    // Adding again should not add another empty item
    model->addEmptyItem();
    EXPECT_EQ(model->rowCount(), initialCount + 1);
}

TEST_F(UT_SideBarModel, CanDropMimeData_InvalidParameters)
{
    QMimeData *data = new QMimeData();

    // Invalid row/column
    bool canDrop = model->canDropMimeData(data, Qt::MoveAction, -1, -1, QModelIndex());
    EXPECT_FALSE(canDrop);

    // Null data
    canDrop = model->canDropMimeData(nullptr, Qt::MoveAction, 0, 0, QModelIndex());
    EXPECT_FALSE(canDrop);

    delete data;
}

TEST_F(UT_SideBarModel, CanDropMimeData_OnSeparator)
{
    SideBarItemSeparator *separator = new SideBarItemSeparator(DefaultGroup::kCommon);
    model->appendRow(separator);

    QMimeData *data = new QMimeData();

    stub.set_lamda(static_cast<SideBarItem *(SideBarModel::*)(int, const QModelIndex &) const>(&SideBarModel::itemFromIndex),
                   [separator](const SideBarModel *, int, const QModelIndex &) -> SideBarItem * {
                       __DBG_STUB_INVOKE__
                       return separator;
                   });

    bool canDrop = model->canDropMimeData(data, Qt::MoveAction, 0, 0, QModelIndex());
    EXPECT_FALSE(canDrop);   // Cannot drop on separator

    delete data;
}

TEST_F(UT_SideBarModel, MimeData)
{
    SideBarItemSeparator *separator = new SideBarItemSeparator(DefaultGroup::kCommon);
    model->appendRow(separator);

    QUrl testUrl = QUrl::fromLocalFile("/home/mime");
    SideBarItem *item = new SideBarItem(testUrl);
    item->setGroup(DefaultGroup::kCommon);
    separator->appendRow(item);

    QModelIndex itemIndex = item->index();
    QModelIndexList indexes;
    indexes << itemIndex;

    stub.set_lamda(VADDR(QStandardItemModel, mimeData),
                   [](const QStandardItemModel *, const QModelIndexList &) -> QMimeData * {
                       __DBG_STUB_INVOKE__
                       return new QMimeData();
                   });

    QMimeData *mimeData = model->mimeData(indexes);

    EXPECT_NE(mimeData, nullptr);

    delete mimeData;
}

TEST_F(UT_SideBarModel, DropMimeData_CannotDrop)
{
    QMimeData *data = new QMimeData();

    stub.set_lamda(VADDR(SideBarModel, canDropMimeData),
                   [](const SideBarModel *, const QMimeData *, Qt::DropAction, int, int, const QModelIndex &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = model->dropMimeData(data, Qt::MoveAction, 0, 0, QModelIndex());
    EXPECT_FALSE(result);

    delete data;
}
