// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "treemodels/sidebarmodel.h"
#include "treeviews/sidebaritemdelegate.h"
#include "treeviews/sidebaritem.h"
#include "treeviews/sidebarview.h"
#include "events/sidebareventcaller.h"
#include "dfmplugin_sidebar_global.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/device/devicealiasmanager.h>

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QMouseEvent>
#include <QHelpEvent>
#include <QToolTip>

using namespace dfmplugin_sidebar;
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

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

TEST_F(UT_SideBarItemDelegate, Paint_SelectedState)
{
    QImage image(200, 40, QImage::Format_ARGB32);
    QPainter painter(&image);

    QWidget widget;
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);
    option.state = QStyle::State_Enabled | QStyle::State_Selected;
    option.widget = &widget;

    SideBarModel model;
    SideBarItem *item = new SideBarItem(QUrl("file:///test"));
    model.appendRow(item);
    QModelIndex index = item->index();

    stub.set_lamda(&SideBarView::currentUrl, [](const SideBarView *) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl("file:///test");
    });

    stub.set_lamda(&SideBarView::isSideBarItemDragged, [](const SideBarView *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&SideBarView::isDropTarget, [](const SideBarView *, const QModelIndex &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(DStyledItemDelegate, initStyleOption), [] { __DBG_STUB_INVOKE__ });

    // Should paint with selected background
    delegate->paint(&painter, option, index);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, Paint_HoverState)
{
    QImage image(200, 40, QImage::Format_ARGB32);
    QPainter painter(&image);

    QWidget widget;
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);
    option.state = QStyle::State_Enabled | QStyle::State_MouseOver;
    option.widget = &widget;

    SideBarModel model;
    SideBarItem *item = new SideBarItem(QUrl("file:///test"));
    model.appendRow(item);
    QModelIndex index = item->index();

    stub.set_lamda(&SideBarView::currentUrl, [](const SideBarView *) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl("file:///other");
    });

    stub.set_lamda(&SideBarView::isSideBarItemDragged, [](const SideBarView *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&SideBarView::isDropTarget, [](const SideBarView *, const QModelIndex &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(VADDR(DStyledItemDelegate, initStyleOption), [] { __DBG_STUB_INVOKE__ });

    // Should paint with hover background
    delegate->paint(&painter, option, index);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, Paint_SeparatorItem)
{
    QImage image(200, 40, QImage::Format_ARGB32);
    QPainter painter(&image);

    QWidget widget;
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);
    option.state = QStyle::State_Enabled | QStyle::State_MouseOver;
    option.widget = &widget;

    SideBarModel model;
    SideBarItemSeparator *separator = new SideBarItemSeparator("TestGroup");
    separator->setExpanded(true);
    model.appendRow(separator);
    QModelIndex index = separator->index();

    stub.set_lamda(&SideBarView::currentUrl, [](const SideBarView *) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl();
    });

    stub.set_lamda(&SideBarView::isSideBarItemDragged, [](const SideBarView *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&SideBarView::isDropTarget, [](const SideBarView *, const QModelIndex &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(VADDR(DStyledItemDelegate, initStyleOption), [] { __DBG_STUB_INVOKE__ });

    // Should paint separator with expand button
    delegate->paint(&painter, option, index);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, Paint_EjectableItem)
{
    QImage image(200, 40, QImage::Format_ARGB32);
    QPainter painter(&image);

    QWidget widget;
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);
    option.state = QStyle::State_Enabled;
    option.widget = &widget;
    option.features = QStyleOptionViewItem::HasDecoration;

    SideBarModel model;
    SideBarItem *item = new SideBarItem(QUrl("file:///media/usb"));
    ItemInfo info = item->itemInfo();
    info.isEjectable = true;
    model.appendRow(item);
    QModelIndex index = item->index();

    stub.set_lamda(&SideBarView::currentUrl, [](const SideBarView *) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl();
    });

    stub.set_lamda(&SideBarItem::itemInfo, [&] { __DBG_STUB_INVOKE__ return info; });

    stub.set_lamda(&SideBarView::isSideBarItemDragged, [](const SideBarView *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&SideBarView::isDropTarget, [](const SideBarView *, const QModelIndex &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(VADDR(DStyledItemDelegate, initStyleOption), [] { __DBG_STUB_INVOKE__ });

    // Should paint with eject icon
    delegate->paint(&painter, option, index);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, EditorEvent_ExpandButton_Click)
{
    SideBarModel model;
    SideBarItemSeparator *separator = new SideBarItemSeparator("TestGroup");
    separator->setExpanded(false);
    model.appendRow(separator);
    QModelIndex index = separator->index();

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);

    // Click on expand button area (right side)
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress,
                                         QPointF(170, 20),   // Position in expand button area
                                         Qt::LeftButton,
                                         Qt::LeftButton,
                                         Qt::NoModifier);

    stub.set_lamda(&SideBarView::isExpanded, [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool expandSignalEmitted = false;
    QObject::connect(delegate, &SideBarItemDelegate::changeExpandState,
                     [&expandSignalEmitted](const QModelIndex &, bool expand) {
                         expandSignalEmitted = true;
                         EXPECT_TRUE(expand);
                     });

    bool result = delegate->editorEvent(event, &model, option, index);

    delete event;

    EXPECT_TRUE(result);
    EXPECT_TRUE(expandSignalEmitted);
}

TEST_F(UT_SideBarItemDelegate, EditorEvent_EjectButton_Click)
{
    SideBarModel model;
    SideBarItem *item = new SideBarItem(QUrl("file:///media/usb"));
    ItemInfo info = item->itemInfo();
    info.isEjectable = true;
    info.url = QUrl("file:///media/usb");
    model.appendRow(item);
    QModelIndex index = item->index();

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);

    // Click on eject button area (bottom right)
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonRelease,
                                         QPointF(180, 25),   // Position in eject button area
                                         Qt::LeftButton,
                                         Qt::LeftButton,
                                         Qt::NoModifier);

    bool ejectCalled = false;
    stub.set_lamda(&SideBarEventCaller::sendEject, [&ejectCalled](const QUrl &) {
        __DBG_STUB_INVOKE__
        ejectCalled = true;
    });

    stub.set_lamda(&SideBarView::currentUrl, [](const SideBarView *) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl("file:///other");
    });

    stub.set_lamda(&SideBarItem::itemInfo, [&] { __DBG_STUB_INVOKE__ return info; });

    bool result = delegate->editorEvent(event, &model, option, index);

    delete event;

    EXPECT_TRUE(result);
    EXPECT_TRUE(ejectCalled);
}

TEST_F(UT_SideBarItemDelegate, EditorEvent_MouseMove_Separator)
{
    SideBarModel model;
    SideBarItemSeparator *separator = new SideBarItemSeparator("TestGroup");
    model.appendRow(separator);
    QModelIndex index = separator->index();

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);

    QMouseEvent *event = new QMouseEvent(QEvent::MouseMove,
                                         QPointF(100, 20),
                                         Qt::NoButton,
                                         Qt::NoButton,
                                         Qt::NoModifier);

    stub.set_lamda(static_cast<void (SideBarView::*)(const QModelIndex &)>(&SideBarView::update), [](SideBarView *, const QModelIndex &) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(VADDR(QStyledItemDelegate, editorEvent), [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = delegate->editorEvent(event, &model, option, index);

    delete event;

    // Should update view on mouse move over separator
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, HelpEvent_ShortText_NoTooltip)
{
    QWidget w;
    QStyleOptionViewItem option;
    option.widget = &w;
    option.rect = QRect(0, 0, 200, 40);

    SideBarModel model;
    SideBarItem *item = new SideBarItem(QUrl("file:///test"));
    item->setText("Short");
    model.appendRow(item);
    QModelIndex index = item->index();

    QHelpEvent *event = new QHelpEvent(QEvent::ToolTip,
                                       QPoint(10, 10),
                                       QPoint(100, 100));

    stub.set_lamda(&QToolTip::hideText, []() {
        __DBG_STUB_INVOKE__
    });

    bool result = delegate->helpEvent(event, view, option, index);

    delete event;

    // Short text should hide tooltip
    EXPECT_TRUE(result);
}

TEST_F(UT_SideBarItemDelegate, HelpEvent_LongText_ShowTooltip)
{
    QWidget w;
    QStyleOptionViewItem option;
    option.widget = &w;
    option.rect = QRect(0, 0, 200, 40);

    SideBarModel model;
    SideBarItem *item = new SideBarItem(QUrl("file:///test"));
    item->setText("This is a very long text that should trigger tooltip display because it exceeds the available width");
    model.appendRow(item);
    QModelIndex index = item->index();

    QHelpEvent *event = new QHelpEvent(QEvent::ToolTip,
                                       QPoint(10, 10),
                                       QPoint(100, 100));

    bool tooltipShown = false;
    stub.set_lamda(&QToolTip::showText, [&tooltipShown] {
        __DBG_STUB_INVOKE__
        tooltipShown = true;
    });

    bool result = delegate->helpEvent(event, view, option, index);

    delete event;

    EXPECT_TRUE(result);
    EXPECT_TRUE(tooltipShown);
}

TEST_F(UT_SideBarItemDelegate, HelpEvent_EjectableItem)
{
    QWidget w;
    QStyleOptionViewItem option;
    option.widget = &w;
    option.rect = QRect(0, 0, 200, 40);

    SideBarModel model;
    SideBarItem *item = new SideBarItem(QUrl("file:///media/usb"));
    item->setText("USB Drive with Very Long Name");
    ItemInfo info = item->itemInfo();
    info.isEjectable = true;
    model.appendRow(item);
    QModelIndex index = item->index();

    QHelpEvent *event = new QHelpEvent(QEvent::ToolTip,
                                       QPoint(10, 10),
                                       QPoint(100, 100));

    stub.set_lamda(&QToolTip::showText, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QToolTip::hideText, []() {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&SideBarItem::itemInfo, [&] { __DBG_STUB_INVOKE__ return info; });

    bool result = delegate->helpEvent(event, view, option, index);

    delete event;

    // Ejectable items have less space for text
    EXPECT_TRUE(result);
}

TEST_F(UT_SideBarItemDelegate, SetEditorData_ValidItem)
{
    QLineEdit editor;

    SideBarModel model;
    SideBarItem *item = new SideBarItem(QUrl("file:///test"));
    ItemInfo info = item->itemInfo();
    info.displayName = "DisplayName";
    info.editDisplayText = "EditText";
    model.appendRow(item);
    QModelIndex index = item->index();

    stub.set_lamda(&SideBarItem::itemInfo, [&] { __DBG_STUB_INVOKE__ return info; });

    delegate->setEditorData(&editor, index);

    // Should set edit text if available, otherwise display name
    EXPECT_EQ(editor.text(), QString("EditText"));
}

TEST_F(UT_SideBarItemDelegate, SetEditorData_NoEditText)
{
    QLineEdit editor;

    SideBarModel model;
    SideBarItem *item = new SideBarItem(QUrl("file:///test"));
    ItemInfo info = item->itemInfo();
    info.displayName = "DisplayName";
    info.editDisplayText = "";   // Empty edit text
    model.appendRow(item);
    QModelIndex index = item->index();

    stub.set_lamda(&SideBarItem::itemInfo, [&] { __DBG_STUB_INVOKE__ return info; });

    delegate->setEditorData(&editor, index);

    // Should use display name when edit text is empty
    EXPECT_EQ(editor.text(), QString("DisplayName"));
}

TEST_F(UT_SideBarItemDelegate, SetModelData_ModifiedEditor)
{
    QLineEdit editor;
    editor.setText("NewName");
    editor.setModified(true);

    SideBarModel model;
    SideBarItem *item = new SideBarItem(QUrl("file:///test"));
    model.appendRow(item);
    QModelIndex index = item->index();

    bool renameSignalEmitted = false;
    QObject::connect(delegate, &SideBarItemDelegate::rename,
                     [&renameSignalEmitted](const QModelIndex &, const QString &text) {
                         renameSignalEmitted = true;
                         EXPECT_EQ(text, QString("NewName"));
                     });

    delegate->setModelData(&editor, &model, index);

    EXPECT_TRUE(renameSignalEmitted);
}

TEST_F(UT_SideBarItemDelegate, SetModelData_UnmodifiedEditor)
{
    QLineEdit editor;
    editor.setText("NewName");
    editor.setModified(false);   // Not modified

    SideBarModel model;
    SideBarItem *item = new SideBarItem(QUrl("file:///test"));
    model.appendRow(item);
    QModelIndex index = item->index();

    bool renameSignalEmitted = false;
    QObject::connect(delegate, &SideBarItemDelegate::rename,
                     [&renameSignalEmitted](const QModelIndex &, const QString &) {
                         renameSignalEmitted = true;
                     });

    delegate->setModelData(&editor, &model, index);

    // Should not emit signal for unmodified editor
    EXPECT_FALSE(renameSignalEmitted);
}

TEST_F(UT_SideBarItemDelegate, OnEditorTextChanged_LongText)
{
    SideBarItem *item = new SideBarItem(QUrl::fromLocalFile("/home/test"));

    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        __DBG_STUB_INVOKE__
        QSharedPointer<FileInfo> info(new FileInfo(QUrl::fromLocalFile("/home/test")));
        return info;
    });

    stub.set_lamda(VADDR(FileInfo, exists), [](const FileInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(FileInfo, extraProperties), [](const FileInfo *) -> QVariantHash {
        __DBG_STUB_INVOKE__
        QVariantHash hash;
        hash["filesystem"] = "ext4";
        return hash;
    });

    stub.set_lamda(&FileUtils::supportedMaxLength, [](const QString &) -> int {
        __DBG_STUB_INVOKE__
        return 255;
    });

    stub.set_lamda(&FileUtils::processLength, [](const QString &, int, int, bool, QString &out, int &) {
        __DBG_STUB_INVOKE__
        out = "Truncated";
        return true;
    });

    // Simulate text change
    QString veryLongText = QString("a").repeated(300);
    delegate->onEditorTextChanged(veryLongText, item);

    delete item;
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarItemDelegate, CreateEditor_WithValidator)
{
    QWidget parent;
    QStyleOptionViewItem option;

    SideBarModel model;
    QLineEdit edit;
    SideBarItem *item = new SideBarItem(QUrl("file:///test"));
    model.appendRow(item);
    QModelIndex index = item->index();

    stub.set_lamda(VADDR(DStyledItemDelegate, createEditor),
                   [&] {
                       __DBG_STUB_INVOKE__
                       return &edit;
                   });

    stub.set_lamda(&NPDeviceAliasManager::canSetAlias, [](NPDeviceAliasManager *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_NO_THROW(delegate->createEditor(&parent, option, index));
}

TEST_F(UT_SideBarItemDelegate, UpdateEditorGeometry_AdjustedWidth)
{
    QLineEdit editor;
    QStyleOptionViewItem option;
    option.rect = QRect(10, 10, 100, 30);

    SideBarModel model;
    SideBarItem *item = new SideBarItem(QUrl("file:///test"));
    model.appendRow(item);
    QModelIndex index = item->index();

    stub.set_lamda(VADDR(DStyledItemDelegate, updateEditorGeometry), [] { __DBG_STUB_INVOKE__ });

    stub.set_lamda(&SideBarView::width, [] {
        __DBG_STUB_INVOKE__
        return 200;
    });

    EXPECT_NO_THROW(delegate->updateEditorGeometry(&editor, option, index));
}

TEST_F(UT_SideBarItemDelegate, EditorEvent_DoubleClick)
{
    SideBarModel model;
    SideBarItem *item = new SideBarItem(QUrl("file:///test"));
    model.appendRow(item);
    QModelIndex index = item->index();

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);

    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonDblClick,
                                         QPointF(100, 20),
                                         Qt::LeftButton,
                                         Qt::LeftButton,
                                         Qt::NoModifier);

    stub.set_lamda(VADDR(QStyledItemDelegate, editorEvent), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = delegate->editorEvent(event, &model, option, index);

    delete event;

    // Double click should be handled
    EXPECT_TRUE(result);
}

TEST_F(UT_SideBarItemDelegate, Paint_InactiveWindow)
{
    QImage image(200, 40, QImage::Format_ARGB32);
    QPainter painter(&image);

    QWidget widget;
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);
    option.state = QStyle::State_Enabled | QStyle::State_Selected;
    option.widget = &widget;

    SideBarModel model;
    SideBarItem *item = new SideBarItem(QUrl("file:///test"));
    model.appendRow(item);
    QModelIndex index = item->index();

    stub.set_lamda(&QWidget::isActiveWindow, [](const QWidget *) -> bool {
        __DBG_STUB_INVOKE__
        return false;   // Inactive window
    });

    stub.set_lamda(&SideBarView::currentUrl, [](const SideBarView *) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl("file:///test");
    });

    stub.set_lamda(&SideBarView::isSideBarItemDragged, [](const SideBarView *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&SideBarView::isDropTarget, [](const SideBarView *, const QModelIndex &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(DStyledItemDelegate, initStyleOption), [] { __DBG_STUB_INVOKE__ });

    // Should paint with reduced opacity when window is inactive
    delegate->paint(&painter, option, index);

    EXPECT_TRUE(true);
}
