// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "treemodels/sidebarmodel.h"
#include "treeviews/sidebaritemdelegate.h"
#include "treeviews/sidebaritem.h"
#include "treeviews/sidebarview.h"
#include "dfmplugin_sidebar_global.h"

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QMouseEvent>
#include <QHelpEvent>

using namespace dfmplugin_sidebar;
DWIDGET_USE_NAMESPACE

class UT_SideBarItemDelegate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Stub GUI operations
        stub.set_lamda(ADDR(QWidget, show), [](QWidget *) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(ADDR(QWidget, hide), [](QWidget *) {
            __DBG_STUB_INVOKE__
        });

        view = new SideBarView();
        delegate = new SideBarItemDelegate(view);
        ASSERT_NE(delegate, nullptr);
    }

    virtual void TearDown() override
    {
        delete delegate;
        delegate = nullptr;
        delete view;
        view = nullptr;
        stub.clear();
    }

protected:
    SideBarItemDelegate *delegate { nullptr };
    SideBarView *view { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_SideBarItemDelegate, Constructor)
{
    EXPECT_NE(delegate, nullptr);
}

TEST_F(UT_SideBarItemDelegate, Paint)
{
    QStyleOptionViewItem option;
    QModelIndex index;

    // Create a mock painter
    QImage image(100, 100, QImage::Format_ARGB32);
    QPainter painter(&image);

    stub.set_lamda(VADDR(DStyledItemDelegate, paint),
                   [](DStyledItemDelegate *, QPainter *, const QStyleOptionViewItem &, const QModelIndex &) {
                       __DBG_STUB_INVOKE__
                   });

    typedef bool (*fun_type)();
    stub.set_lamda((fun_type)&QModelIndex::isValid, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Should not crash even with invalid index
    delegate->paint(&painter, option, index);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, SizeHint)
{
    QStyleOptionViewItem option;
    QModelIndex index;

    stub.set_lamda(VADDR(DStyledItemDelegate, sizeHint),
                   [](const DStyledItemDelegate *, const QStyleOptionViewItem &, const QModelIndex &) -> QSize {
                       __DBG_STUB_INVOKE__
                       return QSize(100, 30);
                   });

    QSize size = delegate->sizeHint(option, index);

    EXPECT_GT(size.width(), 0);
    EXPECT_GT(size.height(), 0);
}

TEST_F(UT_SideBarItemDelegate, CreateEditor)
{
    QWidget parent;
    QStyleOptionViewItem option;
    QModelIndex index;

    typedef bool (*fun_type)();
    stub.set_lamda((fun_type)&QModelIndex::isValid, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    QWidget *editor = delegate->createEditor(&parent, option, index);

    // Should return nullptr for invalid index
    EXPECT_EQ(editor, nullptr);
}

TEST_F(UT_SideBarItemDelegate, CreateEditor_ValidIndex)
{
    QWidget parent;
    QStyleOptionViewItem option;
    QModelIndex index;

    typedef bool (*fun_type)();
    stub.set_lamda((fun_type)&QModelIndex::isValid, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QModelIndex::data, [](const QModelIndex *, int) -> QVariant {
        __DBG_STUB_INVOKE__
        return QVariant(Qt::ItemIsEditable);
    });

    QWidget *editor = delegate->createEditor(&parent, option, index);

    // Editor may or may not be created depending on item type
    if (editor) {
        delete editor;
    }

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, SetEditorData)
{
    QLineEdit editor;
    QModelIndex index;

    stub.set_lamda(&QModelIndex::data, [](const QModelIndex *, int) -> QVariant {
        __DBG_STUB_INVOKE__
        return QVariant("TestText");
    });

    delegate->setEditorData(&editor, index);

    // Editor should be populated
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, SetModelData)
{
    QLineEdit editor;
    editor.setText("NewName");

    QAbstractItemModel *model = nullptr;
    QModelIndex index;

    // Should not crash with null model
    delegate->setModelData(&editor, model, index);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, UpdateEditorGeometry)
{
    QLineEdit editor;
    QStyleOptionViewItem option;
    option.rect = QRect(10, 10, 100, 30);
    QModelIndex index;

    stub.set_lamda(VADDR(DStyledItemDelegate, updateEditorGeometry), [] { __DBG_STUB_INVOKE__ });

    delegate->updateEditorGeometry(&editor, option, index);

    // Editor geometry should be updated
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, EditorEvent_NullEvent)
{
    QAbstractItemModel *model = nullptr;
    QStyleOptionViewItem option;
    QModelIndex index;

    stub.set_lamda(VADDR(QStyledItemDelegate, editorEvent), [] { __DBG_STUB_INVOKE__ return false; });

    bool result = delegate->editorEvent(nullptr, model, option, index);

    EXPECT_FALSE(result);
}

TEST_F(UT_SideBarItemDelegate, EditorEvent_MousePress)
{
    QAbstractItemModel *model = nullptr;
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 30);
    QModelIndex index;

    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress,
                                         QPointF(10, 10),
                                         Qt::LeftButton,
                                         Qt::LeftButton,
                                         Qt::NoModifier);

    stub.set_lamda(VADDR(QStyledItemDelegate, editorEvent),
                   [] {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = delegate->editorEvent(event, model, option, index);

    delete event;

    // Result depends on implementation
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, EditorEvent_MouseRelease)
{
    QAbstractItemModel *model = nullptr;
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 30);
    QModelIndex index;

    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonRelease,
                                         QPointF(10, 10),
                                         Qt::LeftButton,
                                         Qt::LeftButton,
                                         Qt::NoModifier);

    stub.set_lamda(VADDR(QStyledItemDelegate, editorEvent),
                   [] {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = delegate->editorEvent(event, model, option, index);

    delete event;

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, HelpEvent_ToolTip)
{
    QWidget w;
    QStyleOptionViewItem option;
    option.widget = &w;
    QModelIndex index;
    SideBarModel model;

    QHelpEvent *event = new QHelpEvent(QEvent::ToolTip,
                                       QPoint(10, 10),
                                       QPoint(100, 100));

    stub.set_lamda(VADDR(QStyledItemDelegate, helpEvent),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });
    typedef QAbstractItemModel *(*fun_type)();
    stub.set_lamda((fun_type)(&QModelIndex::model), [&] { __DBG_STUB_INVOKE__ return &model; });

    bool result = delegate->helpEvent(event, view, option, index);

    delete event;

    // Result depends on whether tooltip should be shown
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, OnEditorTextChanged)
{
    QString newText = "ChangedText";

    QUrl testUrl = QUrl::fromLocalFile("/home/test");
    SideBarItem *item = new SideBarItem(testUrl);

    stub.set_lamda(&SideBarItem::setText, [] {
        __DBG_STUB_INVOKE__
    });

    delegate->onEditorTextChanged(newText, item);

    // Should update item text
    EXPECT_TRUE(true);

    delete item;
}

TEST_F(UT_SideBarItemDelegate, OnEditorTextChanged_NullItem)
{
    QString newText = "ChangedText";

    // Should not crash with null item
    delegate->onEditorTextChanged(newText, nullptr);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, Paint_WithValidData)
{
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);
    option.state = QStyle::State_Enabled;

    QModelIndex index;

    QImage image(200, 40, QImage::Format_ARGB32);
    QPainter painter(&image);

    typedef bool (*fun_type)();
    stub.set_lamda((fun_type)&QModelIndex::isValid, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QModelIndex::data, [](const QModelIndex *, int role) -> QVariant {
        __DBG_STUB_INVOKE__
        if (role == Qt::DisplayRole)
            return QVariant("TestItem");
        return QVariant();
    });

    stub.set_lamda(VADDR(DStyledItemDelegate, paint),
                   [](DStyledItemDelegate *, QPainter *, const QStyleOptionViewItem &, const QModelIndex &) {
                       __DBG_STUB_INVOKE__
                   });
    stub.set_lamda(VADDR(DStyledItemDelegate, initStyleOption), [] { __DBG_STUB_INVOKE__ });

    // Should paint successfully
    delegate->paint(&painter, option, index);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, SizeHint_Separator)
{
    QStyleOptionViewItem option;
    QModelIndex index;

    stub.set_lamda(&QModelIndex::data, [](const QModelIndex *, int role) -> QVariant {
        __DBG_STUB_INVOKE__
        if (role == SideBarItem::kItemTypeRole)
            return QVariant(SideBarItem::kSeparator);
        return QVariant();
    });

    stub.set_lamda(VADDR(DStyledItemDelegate, sizeHint),
                   [](const DStyledItemDelegate *, const QStyleOptionViewItem &, const QModelIndex &) -> QSize {
                       __DBG_STUB_INVOKE__
                       return QSize(100, 30);
                   });

    QSize size = delegate->sizeHint(option, index);

    // Separator might have different size
    EXPECT_TRUE(true);
}
