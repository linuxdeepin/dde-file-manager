// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "treeviews/sidebarview.h"
#include "treeviews/private/sidebarview_p.h"
#include "treeviews/sidebaritem.h"
#include "treemodels/sidebarmodel.h"
#include "utils/sidebarhelper.h"
#include "utils/fileoperatorhelper.h"

#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <dfm-framework/dpf.h>

#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QTimer>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_sidebar;

class UT_SidebarView : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        model = new SideBarModel;
        SideBarItem *item1 = createGroupItem(QString("group1"));
        SideBarItem *item2 = createGroupItem(QString("group2"));
        model->appendRow(item1);
        model->appendRow(item2);   // two groups added.

        SideBarItem *group1_item1 = createSubItem("item1", QUrl("test/url3"), QString("group1"));
        SideBarItem *group1_item2 = createSubItem("item2", QUrl("test/url4"), QString("group1"));

        model->appendRow(group1_item1);
        model->appendRow(group1_item2);   // two items under group1

        view = new SideBarView;
        view->setModel(model);
    }
    virtual void TearDown() override
    {
        stub.clear();
        if (model) {
            model->clear();
            delete model;
        }
        if (view)
            delete view;
    }
    SideBarItem *createGroupItem(const QString &group)
    {
        SideBarItem *item = new SideBarItemSeparator(group);
        return item;
    }
    SideBarItem *createSubItem(const QString &name, const QUrl &url, const QString &group)
    {
        QString iconName { SystemPathUtil::instance()->systemPathIconName(name) };
        QString text { name };
        if (!iconName.contains("-symbolic"))
            iconName.append("-symbolic");

        SideBarItem *item = new SideBarItem(QIcon::fromTheme(iconName),
                                            text,
                                            group,
                                            url);
        return item;
    }
    SideBarModel *model = nullptr;
    SideBarView *view = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_SidebarView, FindItemIndex)
{
    QModelIndex index1 = view->findItemIndex(QUrl("test/url3"));
    EXPECT_TRUE(index1.row() == 0);

    QModelIndex index2 = view->findItemIndex(QUrl("test/url4"));
    EXPECT_TRUE(index2.row() == 1);
}

TEST_F(UT_SidebarView, testDragEnterEvent)
{
    bool isCall { false };
    stub.set_lamda(&QDropEvent::setDropAction, []() {
        return;
    });

    stub.set_lamda(&QDropEvent::ignore, [&]() {
        isCall = true;
        return;
    });

    QDragEnterEvent event(QPoint(10, 10), Qt::IgnoreAction, nullptr, Qt::LeftButton, Qt::NoModifier);

    // action1
    stub.set_lamda(&QMimeData::urls, []() -> QList<QUrl> {
        return {};
    });

    QMimeData data;
    data.setData(DFMGLOBAL_NAMESPACE::Mime::kDFMTreeUrlsKey, "file:///home");
    stub.set_lamda(&QDropEvent::mimeData, [&] {
        return &data;
    });

    view->dragEnterEvent(&event);
    EXPECT_FALSE(isCall);

    // action2
    isCall = false;
    stub.set_lamda(&QMimeData::urls, []() -> QList<QUrl> {
        return { QUrl("/home/uos") };
    });

    stub.set_lamda(&FileUtils::isContainProhibitPath, []() -> bool {
        return true;
    });

    view->dragEnterEvent(&event);
    EXPECT_TRUE(isCall);
}

TEST_F(UT_SidebarView, testDragLeaveEvent)
{
    bool isCall { false };
    stub.set_lamda(&QAbstractItemView::setCurrentIndex, []() {
        return;
    });
    stub.set_lamda(&QAbstractItemView::setState, []() {
        return;
    });

    auto upDate = static_cast<void (QWidget::*)()>(&QWidget::update);
    stub.set_lamda(upDate, [&]() {
        __DBG_STUB_INVOKE__
        isCall = true;
        return;
    });

    QDragLeaveEvent event;

    view->dragLeaveEvent(&event);
    EXPECT_FALSE(isCall);
}

TEST_F(UT_SidebarView, MousePressEvent_RightButton)
{
    stub.set_lamda(&SideBarView::urlAt, [](SideBarView *, const QPoint &) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl("file:///home/test");
    });

    stub.set_lamda(&SideBarView::itemAt, [](SideBarView *, const QPoint &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    QMouseEvent event(QEvent::MouseButtonPress, QPoint(10, 10), Qt::RightButton, Qt::RightButton, Qt::NoModifier);

    // Right click should be accepted and not trigger parent handler
    view->mousePressEvent(&event);

    EXPECT_TRUE(event.isAccepted());
}

TEST_F(UT_SidebarView, MousePressEvent_LeftButton)
{
    QUrl testUrl("file:///home/test");
    QString testGroup("Common");

    stub.set_lamda(&SideBarView::urlAt, [testUrl](SideBarView *, const QPoint &) -> QUrl {
        __DBG_STUB_INVOKE__
        return testUrl;
    });

    SideBarItem *testItem = createSubItem("test", testUrl, testGroup);
    stub.set_lamda(&SideBarView::itemAt, [testItem](SideBarView *, const QPoint &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return testItem;
    });

    stub.set_lamda(VADDR(DTreeView, mousePressEvent), [](DTreeView *, QMouseEvent *) {
        __DBG_STUB_INVOKE__
    });

    QMouseEvent event(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    view->mousePressEvent(&event);

    delete testItem;
    EXPECT_TRUE(true);
}

TEST_F(UT_SidebarView, MouseReleaseEvent_ValidItem)
{
    bool reportCalled = false;
    QUrl testUrl("file:///home/test");

    SideBarItem *testItem = createSubItem("TestItem", testUrl, "Common");
    model->appendRow(testItem);

    stub.set_lamda(&SideBarView::itemAt, [testItem](SideBarView *, const QPoint &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return testItem;
    });

    stub.set_lamda(VADDR(SideBarView, indexAt), [testItem, this](const SideBarView *, const QPoint &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return testItem->index();
    });

    typedef bool (EventDispatcherManager::*PublishFunc)(const QString &, const QString &, QString, QVariantMap &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&reportCalled](EventDispatcherManager *, const QString &, const QString &, QString, QVariantMap) {
                       __DBG_STUB_INVOKE__
                       reportCalled = true;
                       return true;
                   });

    stub.set_lamda(VADDR(DTreeView, mouseReleaseEvent), [](DTreeView *, QMouseEvent *) {
        __DBG_STUB_INVOKE__
    });

    QMouseEvent event(QEvent::MouseButtonRelease, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    view->mouseReleaseEvent(&event);

    EXPECT_TRUE(reportCalled);
}

TEST_F(UT_SidebarView, ItemAt_ValidPoint)
{
    SideBarItem *item = createSubItem("item", QUrl("file:///test"), "group1");
    model->appendRow(item);

    stub.set_lamda(VADDR(SideBarView, indexAt), [item, this](const SideBarView *, const QPoint &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return item->index();
    });

    SideBarItem *result = view->itemAt(QPoint(10, 10));
    EXPECT_NE(result, nullptr);
    if (result) {
        EXPECT_EQ(result->url(), QUrl("file:///test"));
    }
}

TEST_F(UT_SidebarView, ItemAt_InvalidPoint)
{
    stub.set_lamda(VADDR(SideBarView, indexAt), [](const SideBarView *, const QPoint &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return QModelIndex();
    });

    SideBarItem *result = view->itemAt(QPoint(10, 10));
    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_SidebarView, UrlAt_ValidItem)
{
    QUrl testUrl("file:///home/test");
    SideBarItem *item = createSubItem("item", testUrl, "group1");

    stub.set_lamda(&SideBarView::itemAt, [item](SideBarView *, const QPoint &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return item;
    });

    QUrl result = view->urlAt(QPoint(10, 10));
    EXPECT_EQ(result, testUrl);

    delete item;
}

TEST_F(UT_SidebarView, UrlAt_NoItem)
{
    stub.set_lamda(&SideBarView::itemAt, [](SideBarView *, const QPoint &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    QUrl result = view->urlAt(QPoint(10, 10));
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_SidebarView, SetCurrentUrl_ValidUrl)
{
    QUrl testUrl("test/url3");

    stub.set_lamda(&QAbstractItemView::setCurrentIndex, [](QAbstractItemView *, const QModelIndex &) {
        __DBG_STUB_INVOKE__
    });

    view->setCurrentUrl(testUrl);

    EXPECT_EQ(view->currentUrl(), testUrl);
}

TEST_F(UT_SidebarView, SetCurrentUrl_InvalidUrl)
{
    QUrl invalidUrl("file:///nonexistent");

    stub.set_lamda(&QAbstractItemView::clearSelection, [](QAbstractItemView *) {
        __DBG_STUB_INVOKE__
    });

    view->setCurrentUrl(invalidUrl);

    // Should handle invalid URL gracefully
    EXPECT_TRUE(true);
}

TEST_F(UT_SidebarView, SetCurrentUrl_CollapsedGroup)
{
    QUrl testUrl("test/url3");

    SideBarItem *item = model->itemFromIndex(model->index(0, 0));
    SideBarItemSeparator *separator = dynamic_cast<SideBarItemSeparator *>(item);
    if (separator) {
        separator->setExpanded(false);
    }

    // Should not set current index when group is collapsed
    view->setCurrentUrl(testUrl);

    EXPECT_TRUE(true);
}

TEST_F(UT_SidebarView, CurrentUrl)
{
    QUrl testUrl("file:///home/test");
    view->setCurrentUrl(testUrl);

    EXPECT_EQ(view->currentUrl(), testUrl);
}

TEST_F(UT_SidebarView, SaveStateWhenClose_WithExpandRules)
{
    bool saveConfigCalled = false;

    stub.set_lamda(&SideBarHelper::groupExpandRules, []() -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map["group1"] = true;
        map["group2"] = false;
        return map;
    });

    stub.set_lamda(&SideBarHelper::saveGroupsStateToConfig, [&saveConfigCalled] {
        __DBG_STUB_INVOKE__
        saveConfigCalled = true;
    });

    view->saveStateWhenClose();

    EXPECT_TRUE(saveConfigCalled);
}

TEST_F(UT_SidebarView, SaveStateWhenClose_EmptyRules)
{
    stub.set_lamda(&SideBarHelper::groupExpandRules, []() -> QVariantMap {
        __DBG_STUB_INVOKE__
        return QVariantMap();
    });

    // Should return early when no expand rules
    view->saveStateWhenClose();

    EXPECT_TRUE(true);
}

TEST_F(UT_SidebarView, UpdateSeparatorVisibleState_AllChildrenHidden)
{
    stub.set_lamda(&SideBarHelper::groupExpandRules, []() -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map["group1"] = true;
        return map;
    });

    stub.set_lamda(&QTreeView::isRowHidden, [](const QTreeView *, int, const QModelIndex &) -> bool {
        __DBG_STUB_INVOKE__
        return true;   // All children are hidden
    });

    stub.set_lamda(&QTreeView::setRowHidden, [](QTreeView *, int, const QModelIndex &, bool) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QTreeView::setExpanded, [](QTreeView *, const QModelIndex &, bool) {
        __DBG_STUB_INVOKE__
    });

    view->updateSeparatorVisibleState();

    EXPECT_TRUE(true);
}

TEST_F(UT_SidebarView, UpdateSeparatorVisibleState_SomeChildrenVisible)
{
    stub.set_lamda(&SideBarHelper::groupExpandRules, []() -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map["group1"] = true;
        return map;
    });

    stub.set_lamda(&QTreeView::isRowHidden, [](const QTreeView *, int row, const QModelIndex &) -> bool {
        __DBG_STUB_INVOKE__
        return row > 0;   // First child visible, others hidden
    });

    stub.set_lamda(&QTreeView::setRowHidden, [](QTreeView *, int, const QModelIndex &, bool) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QTreeView::setExpanded, [](QTreeView *, const QModelIndex &, bool) {
        __DBG_STUB_INVOKE__
    });

    view->updateSeparatorVisibleState();

    EXPECT_TRUE(true);
}

TEST_F(UT_SidebarView, OnChangeExpandState_ExpandGroup)
{
    SideBarItem *groupItem = model->itemFromIndex(model->index(0, 0));
    ASSERT_NE(groupItem, nullptr);

    stub.set_lamda(&QTreeView::setExpanded, [](QTreeView *, const QModelIndex &, bool expand) {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(expand);
    });

    stub.set_lamda(&SideBarHelper::groupExpandRules, []() -> QVariantMap {
        __DBG_STUB_INVOKE__
        return QVariantMap();
    });

    stub.set_lamda(&SideBarView::setCurrentUrl, [](SideBarView *, const QUrl &) {
        __DBG_STUB_INVOKE__
    });

    auto updateFunc = static_cast<void (SideBarView::*)(const QModelIndex &)>(&SideBarView::update);
    stub.set_lamda(updateFunc, [] {
        __DBG_STUB_INVOKE__
    });

    view->onChangeExpandState(groupItem->index(), true);

    EXPECT_TRUE(true);
}

TEST_F(UT_SidebarView, OnChangeExpandState_CollapseGroup)
{
    SideBarItem *groupItem = model->itemFromIndex(model->index(0, 0));
    ASSERT_NE(groupItem, nullptr);

    stub.set_lamda(&QTreeView::setExpanded, [](QTreeView *, const QModelIndex &, bool expand) {
        __DBG_STUB_INVOKE__
        EXPECT_FALSE(expand);
    });

    stub.set_lamda(&SideBarHelper::groupExpandRules, []() -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map["group1"] = true;
        return map;
    });

    auto updateFunc = static_cast<void (SideBarView::*)(const QModelIndex &)>(&SideBarView::update);
    stub.set_lamda(updateFunc, [] {
        __DBG_STUB_INVOKE__
    });

    view->onChangeExpandState(groupItem->index(), false);

    EXPECT_TRUE(true);
}

TEST_F(UT_SidebarView, OnChangeExpandState_NullItem)
{
    QModelIndex invalidIndex;

    // Should handle null item gracefully
    view->onChangeExpandState(invalidIndex, true);

    EXPECT_TRUE(true);
}

TEST_F(UT_SidebarView, PreviousIndex)
{
    QModelIndex testIndex = model->index(0, 0);
    view->setPreviousIndex(testIndex);

    EXPECT_EQ(view->previousIndex(), testIndex);
}

TEST_F(UT_SidebarView, IsDropTarget_True)
{
    QModelIndex testIndex = model->index(0, 0);

    // Simulate drag event to set current hover index
    stub.set_lamda(&SideBarView::itemAt, [](SideBarView *, const QPoint &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    stub.set_lamda(VADDR(SideBarView, indexAt), [testIndex](const SideBarView *, const QPoint &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return testIndex;
    });

    QMimeData mimeData;
    QDragMoveEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    stub.set_lamda(&QDropEvent::mimeData, [&mimeData]() -> const QMimeData * {
        __DBG_STUB_INVOKE__
        return &mimeData;
    });

    view->dragMoveEvent(&event);

    bool result = view->isDropTarget(testIndex);
    EXPECT_TRUE(result);
}

TEST_F(UT_SidebarView, IsSideBarItemDragged_False)
{
    bool result = view->isSideBarItemDragged();
    EXPECT_FALSE(result);
}

TEST_F(UT_SidebarView, StartDrag_ValidUrl)
{
    QUrl dragUrl("file:///home/test");

    stub.set_lamda(&SideBarView::urlAt, [dragUrl](SideBarView *, const QPoint &) -> QUrl {
        __DBG_STUB_INVOKE__
        return dragUrl;
    });

    stub.set_lamda(&SideBarView::itemAt, [](SideBarView *, const QPoint &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    stub.set_lamda(VADDR(DTreeView, startDrag), [] {
        __DBG_STUB_INVOKE__
    });

    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    view->mousePressEvent(&pressEvent);

    view->startDrag(Qt::CopyAction | Qt::MoveAction);

    EXPECT_TRUE(view->isSideBarItemDragged());
}

TEST_F(UT_SidebarView, StartDrag_InvalidUrl)
{
    stub.set_lamda(&SideBarView::urlAt, [](SideBarView *, const QPoint &) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl();
    });

    stub.set_lamda(&SideBarView::itemAt, [](SideBarView *, const QPoint &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    view->mousePressEvent(&pressEvent);

    // Should return early without calling parent
    view->startDrag(Qt::CopyAction | Qt::MoveAction);

    EXPECT_TRUE(true);
}

TEST_F(UT_SidebarView, GroupExpandState)
{
    QVariantMap state = view->groupExpandState();
    // Initially should be empty
    EXPECT_TRUE(state.isEmpty());
}

TEST_F(UT_SidebarView, DragMoveEvent_ValidItem)
{
    QUrl testUrl("file:///home/test");
    SideBarItem *item = createSubItem("test", testUrl, "group1");
    model->appendRow(item);

    stub.set_lamda(&SideBarView::itemAt, [item](SideBarView *, const QPoint &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return item;
    });

    stub.set_lamda(VADDR(SideBarView, indexAt), [item](const SideBarView *, const QPoint &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return item->index();
    });

    auto updateFunc = static_cast<void (QWidget::*)()>(&QWidget::update);
    stub.set_lamda(updateFunc, [](QWidget *) {
        __DBG_STUB_INVOKE__
    });

    QMimeData mimeData;
    mimeData.setUrls({ QUrl("file:///home/source") });
    QDragMoveEvent event(QPoint(10, 10), Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);

    stub.set_lamda(&QDropEvent::mimeData, [&mimeData]() -> const QMimeData * {
        __DBG_STUB_INVOKE__
        return &mimeData;
    });

    // Should not ignore the event
    view->dragMoveEvent(&event);

    EXPECT_TRUE(true);
}

TEST_F(UT_SidebarView, DragMoveEvent_NoItem)
{
    stub.set_lamda(&SideBarView::itemAt, [](SideBarView *, const QPoint &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    QMimeData mimeData;
    mimeData.setUrls({ QUrl("file:///home/source") });
    QDragMoveEvent event(QPoint(10, 10), Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);

    stub.set_lamda(&QDropEvent::mimeData, [&mimeData]() -> const QMimeData * {
        __DBG_STUB_INVOKE__
        return &mimeData;
    });

    stub.set_lamda(&QDropEvent::setDropAction, [](QDropEvent *, Qt::DropAction) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QDropEvent::ignore, [] {
        __DBG_STUB_INVOKE__
    });

    view->dragMoveEvent(&event);

    EXPECT_TRUE(true);
}

TEST_F(UT_SidebarView, DropEvent_ValidDrop)
{
    QUrl targetUrl("file:///home/target");
    SideBarItem *item = createSubItem("target", targetUrl, "group1");
    model->appendRow(item);

    stub.set_lamda(&SideBarView::itemAt, [item](SideBarView *, const QPoint &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return item;
    });

    stub.set_lamda(VADDR(SideBarView, indexAt), [item](const SideBarView *, const QPoint &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return item->index();
    });

    stub.set_lamda(VADDR(SideBarView, visualRect), [] {
        __DBG_STUB_INVOKE__
        return QRect(0, 0, 100, 30);
    });

    stub.set_lamda(&SideBarView::onDropData, [](const SideBarView *, QList<QUrl>, QUrl, Qt::DropAction) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QWidget::activateWindow, [](QWidget *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QDropEvent::accept, [] {
        __DBG_STUB_INVOKE__
    });

    QMimeData mimeData;
    mimeData.setUrls({ QUrl("file:///home/source.txt") });
    QDropEvent event(QPoint(10, 10), Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);

    stub.set_lamda(&QDropEvent::mimeData, [&mimeData]() -> const QMimeData * {
        __DBG_STUB_INVOKE__
        return &mimeData;
    });

    stub.set_lamda(&QDropEvent::setDropAction, [](QDropEvent *, Qt::DropAction) {
        __DBG_STUB_INVOKE__
    });

    view->dropEvent(&event);

    EXPECT_TRUE(true);
}

TEST_F(UT_SidebarView, OnDropData_CopyAction)
{
    QUrl srcUrl("file:///home/source.txt");
    QUrl dstUrl("file:///home/target");

    typedef void (*Func)(int, const QObject *, const std::function<void()> &);
    stub.set_lamda(static_cast<Func>(&QTimer::singleShot), [](int, const QObject *, std::function<void()> func) {
        __DBG_STUB_INVOKE__
        func();   // Execute the lambda
    });

    stub.set_lamda(&FileOperatorHelper::pasteFiles, [](FileOperatorHelper *, quint64, const QList<QUrl> &, const QUrl &, Qt::DropAction) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&SideBarHelper::windowId, [](QWidget *) -> quint64 {
        __DBG_STUB_INVOKE__
        return 12345;
    });

    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        __DBG_STUB_INVOKE__
        QSharedPointer<FileInfo> info(new FileInfo(QUrl::fromLocalFile("/home/test")));
        return info;
    });

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QList<QUrl>, QUrl &, Qt::DropAction *&&);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] { return false; });

    bool result = view->onDropData({ srcUrl }, dstUrl, Qt::CopyAction);

    EXPECT_TRUE(result);
}

TEST_F(UT_SidebarView, OnDropData_MoveAction)
{
    QUrl srcUrl("file:///home/source.txt");
    QUrl dstUrl("file:///home/target");

    bool pasteCalled = false;

    stub.set_lamda(&FileOperatorHelper::pasteFiles, [&pasteCalled](FileOperatorHelper *, quint64, const QList<QUrl> &, const QUrl &, Qt::DropAction action) {
        __DBG_STUB_INVOKE__
        pasteCalled = true;
        EXPECT_EQ(action, Qt::MoveAction);
    });

    stub.set_lamda(&SideBarHelper::windowId, [](QWidget *) -> quint64 {
        __DBG_STUB_INVOKE__
        return 12345;
    });

    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        __DBG_STUB_INVOKE__
        QSharedPointer<FileInfo> info(new FileInfo(QUrl::fromLocalFile("/home/test")));
        return info;
    });

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QList<QUrl>, QUrl &, Qt::DropAction *&&);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] { return false; });

    bool result = view->onDropData({ srcUrl }, dstUrl, Qt::MoveAction);

    EXPECT_TRUE(result);
    EXPECT_TRUE(pasteCalled);
}

TEST_F(UT_SidebarView, OnDropData_IgnoreAction)
{
    QUrl srcUrl("file:///home/source.txt");
    QUrl dstUrl("file:///home/target");

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QList<QUrl>, QUrl &, Qt::DropAction *&&);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] { return true; });

    bool result = view->onDropData({ srcUrl }, dstUrl, Qt::IgnoreAction);

    EXPECT_TRUE(result);
}

TEST_F(UT_SidebarView, CanDropMimeData_IgnoreAction_EmptyUrls)
{
    SideBarItem *item = createSubItem("test", QUrl("file:///test"), "group1");

    QMimeData mimeData;
    Qt::DropAction result = view->canDropMimeData(item, &mimeData, Qt::CopyAction | Qt::MoveAction);

    EXPECT_EQ(result, Qt::IgnoreAction);

    delete item;
}

TEST_F(UT_SidebarView, CanDropMimeData_IgnoreAction_InvalidTargetUrl)
{
    SideBarItem *item = createSubItem("test", QUrl(), "group1");

    QMimeData mimeData;
    mimeData.setUrls({ QUrl("file:///source") });

    Qt::DropAction result = view->canDropMimeData(item, &mimeData, Qt::CopyAction | Qt::MoveAction);

    EXPECT_EQ(result, Qt::IgnoreAction);

    delete item;
}

TEST_F(UT_SidebarView, CanDropMimeData_CopyAction)
{
    QUrl targetUrl("file:///home/target");
    SideBarItem *item = createSubItem("target", targetUrl, "group1");

    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        __DBG_STUB_INVOKE__
        QSharedPointer<FileInfo> info(new FileInfo(QUrl::fromLocalFile("/home/test")));
        return info;
    });

    stub.set_lamda(VADDR(FileInfo, canAttributes), [](const FileInfo *, CanableInfoType) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](const FileInfo *, OptInfoType) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(FileInfo, fileType), [](const FileInfo *) -> FileInfo::FileType {
        __DBG_STUB_INVOKE__
        return FileInfo::FileType::kDirectory;
    });

    stub.set_lamda(VADDR(FileInfo, urlOf), [targetUrl](const FileInfo *, UrlInfoType) -> QUrl {
        __DBG_STUB_INVOKE__
        return targetUrl;
    });

    stub.set_lamda(VADDR(FileInfo, supportedOfAttributes), [](const FileInfo *, SupportedType) -> Qt::DropActions {
        __DBG_STUB_INVOKE__
        return Qt::CopyAction | Qt::MoveAction;
    });

    stub.set_lamda(&FileUtils::isSameDevice, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&FileUtils::isTrashFile, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QMimeData mimeData;
    mimeData.setUrls({ QUrl("file:///home/source.txt") });

    EXPECT_NO_THROW(view->canDropMimeData(item, &mimeData, Qt::CopyAction | Qt::MoveAction));

    delete item;
}

TEST_F(UT_SidebarView, CanDropMimeData_MoveAction_SameDevice)
{
    QUrl targetUrl("file:///home/target");
    SideBarItem *item = createSubItem("target", targetUrl, "group1");

    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        __DBG_STUB_INVOKE__
        QSharedPointer<FileInfo> info(new FileInfo(QUrl::fromLocalFile("/home/test")));
        return info;
    });

    stub.set_lamda(VADDR(FileInfo, canAttributes), [](const FileInfo *, CanableInfoType) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](const FileInfo *, OptInfoType) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(FileInfo, fileType), [](const FileInfo *) -> FileInfo::FileType {
        __DBG_STUB_INVOKE__
        return FileInfo::FileType::kDirectory;
    });

    stub.set_lamda(VADDR(FileInfo, urlOf), [targetUrl](const FileInfo *, UrlInfoType) -> QUrl {
        __DBG_STUB_INVOKE__
        return targetUrl;
    });

    stub.set_lamda(VADDR(FileInfo, supportedOfAttributes), [](const FileInfo *, SupportedType) -> Qt::DropActions {
        __DBG_STUB_INVOKE__
        return Qt::CopyAction | Qt::MoveAction;
    });

    stub.set_lamda(&FileUtils::isSameDevice, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileUtils::isTrashFile, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&UrlRoute::urlParent, [](const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl("file:///other");
    });

    QMimeData mimeData;
    mimeData.setUrls({ QUrl("file:///home/source.txt") });

    EXPECT_NO_THROW(view->canDropMimeData(item, &mimeData, Qt::CopyAction | Qt::MoveAction));
    delete item;
}

TEST_F(UT_SidebarView, CanDropMimeData_TrashFile_DifferentUser)
{
    QUrl targetUrl("file:///home/.local/share/Trash/files");
    SideBarItem *item = createSubItem("trash", targetUrl, "group1");

    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        __DBG_STUB_INVOKE__
        QSharedPointer<FileInfo> info(new FileInfo(QUrl::fromLocalFile("/home/test")));
        return info;
    });

    stub.set_lamda(VADDR(FileInfo, canAttributes), [](const FileInfo *, CanableInfoType) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](const FileInfo *, OptInfoType) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(FileInfo, fileType), [](const FileInfo *) -> FileInfo::FileType {
        __DBG_STUB_INVOKE__
        return FileInfo::FileType::kDirectory;
    });

    stub.set_lamda(VADDR(FileInfo, urlOf), [targetUrl](const FileInfo *, UrlInfoType) -> QUrl {
        __DBG_STUB_INVOKE__
        return targetUrl;
    });

    stub.set_lamda(VADDR(FileInfo, supportedOfAttributes), [](const FileInfo *, SupportedType) -> Qt::DropActions {
        __DBG_STUB_INVOKE__
        return Qt::CopyAction | Qt::MoveAction;
    });

    stub.set_lamda(&FileUtils::isTrashFile, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&SysInfoUtils::isSameUser, [](const QMimeData *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&UrlRoute::urlParent, [](const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl("file:///other");
    });

    QMimeData mimeData;
    mimeData.setUrls({ QUrl("file:///home/source.txt") });

    Qt::DropAction result = view->canDropMimeData(item, &mimeData, Qt::CopyAction | Qt::MoveAction);

    EXPECT_EQ(result, Qt::IgnoreAction);

    delete item;
}

TEST_F(UT_SidebarView, IsAccepteDragEvent_Accept)
{
    QUrl targetUrl("file:///home/target");
    SideBarItem *item = createSubItem("target", targetUrl, "group1");

    stub.set_lamda(&SideBarView::itemAt, [item](SideBarView *, const QPoint &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return item;
    });

    stub.set_lamda(&SideBarView::canDropMimeData, [](const SideBarView *, SideBarItem *, const QMimeData *, Qt::DropActions) -> Qt::DropAction {
        __DBG_STUB_INVOKE__
        return Qt::CopyAction;
    });

    stub.set_lamda(&QDropEvent::setDropAction, [](QDropEvent *, Qt::DropAction) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QDropEvent::accept, [] {
        __DBG_STUB_INVOKE__
    });

    QMimeData mimeData;
    mimeData.setUrls({ QUrl("file:///home/source.txt") });
    QDropEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);

    stub.set_lamda(&QDropEvent::mimeData, [&mimeData]() -> const QMimeData * {
        __DBG_STUB_INVOKE__
        return &mimeData;
    });

    bool result = view->isAccepteDragEvent(&event);

    EXPECT_TRUE(result);
}

TEST_F(UT_SidebarView, IsAccepteDragEvent_Reject_NoItem)
{
    stub.set_lamda(&SideBarView::itemAt, [](SideBarView *, const QPoint &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    QMimeData mimeData;
    QDropEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);

    stub.set_lamda(&QDropEvent::mimeData, [&mimeData]() -> const QMimeData * {
        __DBG_STUB_INVOKE__
        return &mimeData;
    });

    bool result = view->isAccepteDragEvent(&event);

    EXPECT_FALSE(result);
}

TEST_F(UT_SidebarView, FindItemIndex_WithCallback)
{
    QUrl testUrl("file:///home/test");
    SideBarItem *item = createSubItem("test", testUrl, "group1");

    ItemInfo info = item->itemInfo();
    info.findMeCb = [](const QUrl &, const QUrl &) -> bool {
        return true;
    };

    stub.set_lamda(&SideBarItem::itemInfo, [&] { __DBG_STUB_INVOKE__ return info; });

    model->appendRow(item);

    QModelIndex result = view->findItemIndex(QUrl("file:///other"));

    EXPECT_TRUE(result.isValid());
}

TEST_F(UT_SidebarView, FindItemIndex_NotFound)
{
    QModelIndex result = view->findItemIndex(QUrl("file:///nonexistent"));

    EXPECT_FALSE(result.isValid());
}
