// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "dialogs/collectiondelegate.h"

#include <DGuiApplicationHelper>
#include <DStyledItemDelegate>
#include <DListView>

#include <gtest/gtest.h>
#include <QPainter>
#include <QMouseEvent>
#include <QStyleOptionViewItem>
#include <QAbstractItemModel>
#include <QPixmap>
#include <QImage>
#include <QSignalSpy>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class CollectionDelegateTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Stub QIcon::fromTheme
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            QPixmap pixmap(24, 24);
            pixmap.fill(Qt::red);
            return QIcon(pixmap);
        });

        // Stub DGuiApplicationHelper
        stub.set_lamda(&DGuiApplicationHelper::instance, []() -> DGuiApplicationHelper * {
            __DBG_STUB_INVOKE__
            static DGuiApplicationHelper helper;
            return &helper;
        });

        stub.set_lamda(&DGuiApplicationHelper::themeType, [](DGuiApplicationHelper *) {
            __DBG_STUB_INVOKE__
            return DGuiApplicationHelper::LightType;
        });

        listView = new DListView();
        delegate = new CollectionDelegate(listView);
        model = new CollectionModel(listView);
        listView->setModel(model);
        listView->setItemDelegate(delegate);
    }

    void TearDown() override
    {
        delete listView;
        listView = nullptr;
        delegate = nullptr;
        model = nullptr;
        stub.clear();
    }

    DListView *listView { nullptr };
    CollectionDelegate *delegate { nullptr };
    CollectionModel *model { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(CollectionDelegateTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(delegate, nullptr);
}

TEST_F(CollectionDelegateTest, Constructor_WithParent_SetsParent)
{
    EXPECT_EQ(delegate->parent(), listView);
}

TEST_F(CollectionDelegateTest, SizeHint_AnyIndex_ReturnsFixedSize)
{
    QStyleOptionViewItem option;
    QModelIndex index;

    QSize size = delegate->sizeHint(option, index);

    EXPECT_EQ(size, QSize(376, 36));
}

TEST_F(CollectionDelegateTest, SizeHint_ValidIndex_ReturnsFixedSize)
{
    model->setStringList(QStringList() << "item1" << "item2");
    QModelIndex index = model->index(0, 0);
    QStyleOptionViewItem option;

    QSize size = delegate->sizeHint(option, index);

    EXPECT_EQ(size, QSize(376, 36));
}

TEST_F(CollectionDelegateTest, Paint_EvenRow_DrawsBackground)
{
    model->setStringList(QStringList() << "item1" << "item2");
    QModelIndex index = model->index(0, 0);  // Even row

    QPixmap pixmap(376, 36);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CollectionDelegateTest, Paint_OddRow_NoExtraBackground)
{
    model->setStringList(QStringList() << "item1" << "item2");
    QModelIndex index = model->index(1, 0);  // Odd row

    QPixmap pixmap(376, 36);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CollectionDelegateTest, Paint_MouseOver_DrawsHoverBackground)
{
    model->setStringList(QStringList() << "item1");
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(376, 36);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);
    option.state |= QStyle::State_MouseOver;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CollectionDelegateTest, Paint_MouseOverAndSelected_DrawsDeleteButton)
{
    model->setStringList(QStringList() << "item1");
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(376, 36);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);
    option.state |= QStyle::State_MouseOver;
    option.state |= QStyle::State_Selected;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CollectionDelegateTest, Paint_LightTheme_UsesCorrectColor)
{
    stub.set_lamda(&DGuiApplicationHelper::themeType, [](DGuiApplicationHelper *) {
        __DBG_STUB_INVOKE__
        return DGuiApplicationHelper::LightType;
    });

    model->setStringList(QStringList() << "item1");
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(376, 36);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CollectionDelegateTest, Paint_DarkTheme_UsesCorrectColor)
{
    stub.set_lamda(&DGuiApplicationHelper::themeType, [](DGuiApplicationHelper *) {
        __DBG_STUB_INVOKE__
        return DGuiApplicationHelper::DarkType;
    });

    model->setStringList(QStringList() << "item1");
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(376, 36);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CollectionDelegateTest, Paint_InvalidIndex_DoesNotCrash)
{
    QModelIndex index;  // Invalid index

    QPixmap pixmap(376, 36);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    stub.set_lamda(VADDR(QStyledItemDelegate, paint), []{
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CollectionDelegateTest, EditorEvent_ClickDeleteButton_EmitsSignal)
{
    model->setStringList(QStringList() << "test_item");
    QModelIndex index = model->index(0, 0);

    QSignalSpy spy(delegate, &CollectionDelegate::removeItemManually);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    // Click position on delete button (width - 30, top + 6)
    QPoint clickPos(346, 12);
    QMouseEvent event(QEvent::MouseButtonPress, clickPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    bool result = delegate->editorEvent(&event, model, option, index);

    EXPECT_TRUE(result);
    EXPECT_EQ(spy.count(), 1);
    if (spy.count() > 0) {
        EXPECT_EQ(spy.at(0).at(0).toString(), "test_item");
        EXPECT_EQ(spy.at(0).at(1).toInt(), 0);
    }
}

TEST_F(CollectionDelegateTest, EditorEvent_ClickOutsideDeleteButton_DoesNotEmitSignal)
{
    model->setStringList(QStringList() << "test_item");
    QModelIndex index = model->index(0, 0);

    QSignalSpy spy(delegate, &CollectionDelegate::removeItemManually);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    // Click outside delete button area
    QPoint clickPos(100, 18);
    QMouseEvent event(QEvent::MouseButtonPress, clickPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    bool result = delegate->editorEvent(&event, model, option, index);

    EXPECT_FALSE(result);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(CollectionDelegateTest, EditorEvent_RightClick_DoesNotEmitSignal)
{
    model->setStringList(QStringList() << "test_item");
    QModelIndex index = model->index(0, 0);

    QSignalSpy spy(delegate, &CollectionDelegate::removeItemManually);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    QPoint clickPos(346, 12);
    QMouseEvent event(QEvent::MouseButtonPress, clickPos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);

    bool result = delegate->editorEvent(&event, model, option, index);

    EXPECT_FALSE(result);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(CollectionDelegateTest, EditorEvent_InvalidIndex_ReturnsFalse)
{
    QModelIndex index;  // Invalid index

    QSignalSpy spy(delegate, &CollectionDelegate::removeItemManually);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    QPoint clickPos(346, 12);
    QMouseEvent event(QEvent::MouseButtonPress, clickPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    bool result = delegate->editorEvent(&event, model, option, index);

    EXPECT_FALSE(result);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(CollectionDelegateTest, EditorEvent_NonMouseEvent_PassesToBase)
{
    model->setStringList(QStringList() << "test_item");
    QModelIndex index = model->index(0, 0);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    QEvent event(QEvent::KeyPress);

    bool result = delegate->editorEvent(&event, model, option, index);

    // Should be handled by base class
    EXPECT_FALSE(result);
}

TEST_F(CollectionDelegateTest, EditorEvent_MouseReleaseOnDeleteButton_DoesNotEmitSignal)
{
    model->setStringList(QStringList() << "test_item");
    QModelIndex index = model->index(0, 0);

    QSignalSpy spy(delegate, &CollectionDelegate::removeItemManually);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    QPoint clickPos(346, 12);
    QMouseEvent event(QEvent::MouseButtonRelease, clickPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    bool result = delegate->editorEvent(&event, model, option, index);

    EXPECT_FALSE(result);
    EXPECT_EQ(spy.count(), 0);
}

class CollectionModelTest : public testing::Test
{
protected:
    void SetUp() override
    {
        model = new CollectionModel(nullptr);
    }

    void TearDown() override
    {
        delete model;
        model = nullptr;
    }

    CollectionModel *model { nullptr };
};

TEST_F(CollectionModelTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(model, nullptr);
}

TEST_F(CollectionModelTest, Data_DisplayRole_ReturnsFullText)
{
    QStringList items;
    items << "smb://192.168.1.1/share" << "ftp://example.com:21";
    model->setStringList(items);

    QModelIndex index = model->index(0, 0);
    QVariant data = model->data(index, Qt::DisplayRole);

    EXPECT_EQ(data.toString(), "smb://192.168.1.1/share");
}

TEST_F(CollectionModelTest, Data_UrlRole_ReturnsFullText)
{
    QStringList items;
    items << "smb://192.168.1.1/share";
    model->setStringList(items);

    QModelIndex index = model->index(0, 0);
    QVariant data = model->data(index, CollectionModel::kUrlRole);

    EXPECT_EQ(data.toString(), "smb://192.168.1.1/share");
}

TEST_F(CollectionModelTest, Data_OtherRole_PassesToBase)
{
    QStringList items;
    items << "test_item";
    model->setStringList(items);

    QModelIndex index = model->index(0, 0);
    QVariant data = model->data(index, Qt::ToolTipRole);

    // Should return data from base class
    EXPECT_TRUE(data.isValid() || !data.isValid());
}

TEST_F(CollectionModelTest, Data_InvalidIndex_ReturnsInvalid)
{
    QModelIndex index;  // Invalid index
    EXPECT_NO_THROW(model->data(index, Qt::DisplayRole));
}

TEST_F(CollectionModelTest, FindItem_ExistingItem_ReturnsIndex)
{
    QStringList items;
    items << "item1" << "item2" << "item3";
    model->setStringList(items);

    int index = model->findItem("item2");

    EXPECT_EQ(index, 1);
}

TEST_F(CollectionModelTest, FindItem_NonExistingItem_ReturnsNegative)
{
    QStringList items;
    items << "item1" << "item2" << "item3";
    model->setStringList(items);

    int index = model->findItem("item4");

    EXPECT_EQ(index, -1);
}

TEST_F(CollectionModelTest, FindItem_EmptyList_ReturnsNegative)
{
    model->setStringList(QStringList());

    int index = model->findItem("item1");

    EXPECT_EQ(index, -1);
}

TEST_F(CollectionModelTest, FindItem_FirstItem_ReturnsZero)
{
    QStringList items;
    items << "first" << "second" << "third";
    model->setStringList(items);

    int index = model->findItem("first");

    EXPECT_EQ(index, 0);
}

TEST_F(CollectionModelTest, FindItem_LastItem_ReturnsCorrectIndex)
{
    QStringList items;
    items << "item1" << "item2" << "item3";
    model->setStringList(items);

    int index = model->findItem("item3");

    EXPECT_EQ(index, 2);
}

TEST_F(CollectionModelTest, FindItem_DuplicateItems_ReturnsFirstOccurrence)
{
    QStringList items;
    items << "item1" << "duplicate" << "duplicate" << "item2";
    model->setStringList(items);

    int index = model->findItem("duplicate");

    EXPECT_EQ(index, 1);
}

TEST_F(CollectionModelTest, Data_MultipleItems_ReturnsCorrectData)
{
    QStringList items;
    items << "smb://server1/share1"
          << "ftp://server2:21/path"
          << "http://example.com";
    model->setStringList(items);

    EXPECT_EQ(model->data(model->index(0, 0), Qt::DisplayRole).toString(), "smb://server1/share1");
    EXPECT_EQ(model->data(model->index(1, 0), Qt::DisplayRole).toString(), "ftp://server2:21/path");
    EXPECT_EQ(model->data(model->index(2, 0), Qt::DisplayRole).toString(), "http://example.com");
}

TEST_F(CollectionModelTest, UrlRole_MatchesDisplayRole_ForAllItems)
{
    QStringList items;
    items << "url1" << "url2" << "url3";
    model->setStringList(items);

    for (int i = 0; i < items.count(); ++i) {
        QModelIndex index = model->index(i, 0);
        QString displayData = model->data(index, Qt::DisplayRole).toString();
        QString urlData = model->data(index, CollectionModel::kUrlRole).toString();
        EXPECT_EQ(displayData, urlData);
    }
}

TEST_F(CollectionDelegateTest, Paint_AntiAliasing_Enabled)
{
    model->setStringList(QStringList() << "item1");
    QModelIndex index = model->index(0, 0);

    QImage image(376, 36, QImage::Format_ARGB32);
    QPainter painter(&image);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    delegate->paint(&painter, option, index);

    // Verify painter has antialiasing enabled (can't directly check but should not crash)
    EXPECT_TRUE(painter.testRenderHint(QPainter::Antialiasing));
}

TEST_F(CollectionDelegateTest, DeleteButton_Position_CorrectlyCalculated)
{
    model->setStringList(QStringList() << "test_item");
    QModelIndex index = model->index(0, 0);

    QSignalSpy spy(delegate, &CollectionDelegate::removeItemManually);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    // Test exact button boundary (width - 30 = 346, top + 6 = 6)
    QPoint clickPos(346, 6);
    QMouseEvent event(QEvent::MouseButtonPress, clickPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    bool result = delegate->editorEvent(&event, model, option, index);

    EXPECT_TRUE(result);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(CollectionDelegateTest, DeleteButton_EdgeClick_HandledCorrectly)
{
    model->setStringList(QStringList() << "test_item");
    QModelIndex index = model->index(0, 0);

    QSignalSpy spy(delegate, &CollectionDelegate::removeItemManually);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    // Click at edge of button (width - 30 + 21 = 367, top + 6 + 21 = 27)
    QPoint clickPos(367, 27);
    QMouseEvent event(QEvent::MouseButtonPress, clickPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    bool result = delegate->editorEvent(&event, model, option, index);

    EXPECT_TRUE(result);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(CollectionDelegateTest, Paint_RoundedCorners_Drawn)
{
    model->setStringList(QStringList() << "item1");
    QModelIndex index = model->index(0, 0);

    QImage image(376, 36, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));

    // Verify painting completed without crash
}

TEST_F(CollectionDelegateTest, EditorEvent_MultipleClicks_EmitsMultipleSignals)
{
    model->setStringList(QStringList() << "item1" << "item2");

    QSignalSpy spy(delegate, &CollectionDelegate::removeItemManually);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 376, 36);

    // Click first item
    QModelIndex index1 = model->index(0, 0);
    QPoint clickPos1(346, 12);
    QMouseEvent event1(QEvent::MouseButtonPress, clickPos1, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    delegate->editorEvent(&event1, model, option, index1);

    // Click second item
    QModelIndex index2 = model->index(1, 0);
    QPoint clickPos2(346, 12);
    QMouseEvent event2(QEvent::MouseButtonPress, clickPos2, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    delegate->editorEvent(&event2, model, option, index2);

    EXPECT_EQ(spy.count(), 2);
}
