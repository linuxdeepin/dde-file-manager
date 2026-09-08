// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "canvas_test_common.h"

#include "plugins/desktop/ddplugin-canvas/view/canvasview.h"
#include "plugins/desktop/ddplugin-canvas/view/canvasview_p.h"
#include "plugins/desktop/ddplugin-canvas/view/viewhookinterface.h"
#include "plugins/desktop/ddplugin-canvas/view/operator/canvasviewmenuproxy.h"
#include "plugins/desktop/ddplugin-canvas/view/operator/viewsettingutil.h"
#include "plugins/desktop/ddplugin-canvas/view/operator/viewpainter.h"
#include "plugins/desktop/ddplugin-canvas/view/operator/dragdropoper.h"
#include "plugins/desktop/ddplugin-canvas/view/operator/dodgeoper.h"
#include "plugins/desktop/ddplugin-canvas/view/operator/keyselector.h"
#include "plugins/desktop/ddplugin-canvas/view/operator/clickselector.h"
#include "plugins/desktop/ddplugin-canvas/view/operator/boxselector.h"
#include "plugins/desktop/ddplugin-canvas/view/operator/fileoperatorproxy.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasproxymodel.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasselectionmodel.h"
#include "plugins/desktop/ddplugin-canvas/delegate/canvasitemdelegate.h"

#include <dfm-base/utils/windowutils.h>
#include <dfm-base/base/application/application.h>

#include <gtest/gtest.h>
#include <QApplication>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QFocusEvent>
#include <QPaintEvent>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>

using namespace ddplugin_canvas;
DFMBASE_USE_NAMESPACE

namespace {
class TestProxyModel : public CanvasProxyModel
{
public:
    Qt::ItemFlags flags(const QModelIndex &) const override
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    QVariant data(const QModelIndex &, int) const override
    {
        return dataResult;
    }

    QVariant dataResult = QVariant(QString("apple"));
};

class DummyViewHook : public ViewHookInterface
{
public:
    bool keyPress(int, int, int, void * = nullptr) const override { keyPressCount++; return keyPressResult; }
    bool mousePress(int, int, const QPoint &, void * = nullptr) const override { mousePressCount++; return mousePressResult; }
    bool wheel(int, const QPoint &, void * = nullptr) const override { wheelCount++; return wheelResult; }
    bool startDrag(int, int, void * = nullptr) const override { startDragCount++; return startDragResult; }

    mutable int keyPressCount = 0;
    mutable int mousePressCount = 0;
    mutable int wheelCount = 0;
    mutable int startDragCount = 0;
    bool keyPressResult = false;
    bool mousePressResult = false;
    bool wheelResult = false;
    bool startDragResult = false;
};
}   // namespace

class CanvasViewEventTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(VADDR(QWidget, show), [](QWidget *) { __DBG_STUB_INVOKE__; });
        stub.set_lamda(VADDR(QWidget, setVisible), [](QWidget *, bool) { __DBG_STUB_INVOKE__; });
        // CanvasView::reset calls model()->rootIndex() through the proxy cast.
        stub.set_lamda(VADDR(CanvasView, reset), [](CanvasView *) { __DBG_STUB_INVOKE__; });

        view = new CanvasView(nullptr);
        // Valid grid metrics so real CanvasViewPrivate::gridAt never divides by zero.
        view->d->canvasInfo = CanvasViewPrivate::CanvasInfo(2, 2, 100, 100);

        proxyModel = new CanvasProxyModel();
        // CanvasView::selectionModel() qobject_casts to CanvasSelectionModel, so a
        // plain QItemSelectionModel would surface as null inside the view. Qt also
        // requires the selection model to work on the same model as the view.
        mockSelectionModel = new CanvasSelectionModel(proxyModel, proxyModel);
        view->setModel(proxyModel);
        view->setSelectionModel(mockSelectionModel);
        // Plain model only used to fabricate QModelIndex values in tests.
        mockModel = new QStandardItemModel();

        stub.set_lamda(ADDR(CanvasView, model),
                       [this](const CanvasView *) -> CanvasProxyModel * {
                           return proxyModel;
                       });
        view->setItemDelegate(new CanvasItemDelegate(view));

        hook = new DummyViewHook();
        view->setViewHook(hook);
    }

    void TearDown() override
    {
        view->setViewHook(nullptr);
        delete view;
        delete proxyModel;
        delete hook;
        delete mockModel;   // deletes mockSelectionModel as its child
        stub.clear();
    }

    stub_ext::StubExt stub;
    CanvasView *view = nullptr;
    QStandardItemModel *mockModel = nullptr;
    CanvasSelectionModel *mockSelectionModel = nullptr;
    CanvasProxyModel *proxyModel = nullptr;
    DummyViewHook *hook = nullptr;
};

TEST_F(CanvasViewEventTest, ViewHook_SetAndGet_ReturnsSameInterface)
{
    // Arrange
    DummyViewHook otherHook;

    // Act
    view->setViewHook(&otherHook);

    // Assert
    EXPECT_EQ(view->viewHook(), &otherHook);
    view->setViewHook(hook);
    EXPECT_EQ(view->viewHook(), hook);
    EXPECT_NE(view->viewHook(), &otherHook);
}

TEST_F(CanvasViewEventTest, SetSelection_RectAndFlags_SelectionRemainsEmpty)
{
    // Arrange
    QStandardItem *item = new QStandardItem("a");
    mockModel->appendRow(item);
    EXPECT_TRUE(view->selectionModel()->selectedIndexes().isEmpty());

    // Act: CanvasView::setSelection is intentionally a no-op.
    view->setSelection(QRect(0, 0, 100, 100), QItemSelectionModel::ClearAndSelect);

    // Assert
    EXPECT_TRUE(view->selectionModel()->selectedIndexes().isEmpty());
    EXPECT_EQ(view->selectionModel()->selectedIndexes().size(), 0);
}

TEST_F(CanvasViewEventTest, VisualRegionForSelection_EmptySelection_ReturnsEmptyRegion)
{
    // Arrange
    QItemSelection emptySelection;

    // Act
    QRegion region = view->visualRegionForSelection(emptySelection);

    // Assert
    EXPECT_TRUE(region.isEmpty());
    EXPECT_TRUE(region.boundingRect().isNull());
}

TEST_F(CanvasViewEventTest, KeyboardSearch_ValidSearch_ForwardedToSelector)
{
    // Arrange
    QString captured;
    stub.set_lamda(ADDR(KeySelector, keyboardSearch),
                   [&captured](KeySelector *, const QString &search) {
                       captured = search;
                   });

    // Act
    view->keyboardSearch(QString("abc"));

    // Assert
    EXPECT_EQ(captured, QString("abc"));
}

TEST_F(CanvasViewEventTest, PaintEvent_NormalState_AllPaintStagesExecuted)
{
    // Arrange
    bool gridInfosDrawn = false;
    bool moveDrawn = false;
    bool dodgeDrawn = false;
    bool filesPainted = false;
    stub.set_lamda(ADDR(ViewPainter, drawGirdInfos),
                   [&gridInfosDrawn](ViewPainter *) {
                       gridInfosDrawn = true;
                   });
    stub.set_lamda(ADDR(ViewPainter, drawMove),
                   [&moveDrawn](ViewPainter *, QStyleOptionViewItem) {
                       moveDrawn = true;
                   });
    stub.set_lamda(ADDR(ViewPainter, drawDodge),
                   [&dodgeDrawn](ViewPainter *, QStyleOptionViewItem) {
                       dodgeDrawn = true;
                   });
    stub.set_lamda(ADDR(ViewPainter, paintFiles),
                   [&filesPainted](ViewPainter *, QStyleOptionViewItem, QPaintEvent *) {
                       filesPainted = true;
                   });
    view->d->flicker = false;

    // Act
    QPaintEvent event(QRect(0, 0, 400, 300));
    view->paintEvent(&event);

    // Assert
    EXPECT_TRUE(gridInfosDrawn);
    EXPECT_TRUE(moveDrawn);
    EXPECT_TRUE(dodgeDrawn);
    EXPECT_TRUE(filesPainted);
}

TEST_F(CanvasViewEventTest, ContextMenuEvent_EmptyArea_ShowsEmptyAreaMenu)
{
    // Arrange
    stub.set_lamda(ADDR(CanvasViewMenuProxy, disableMenu),
                   []() -> bool {
                       return false;
                   });
    stub.set_lamda(ADDR(WindowUtils, isWayLand),
                   []() -> bool {
                       return false;
                   });
    stub.set_lamda(VADDR(CanvasView, indexAt),
                   [](const CanvasView *, const QPoint &) -> QModelIndex {
                       return QModelIndex();
                   });
    bool editorReverted = false;
    stub.set_lamda(ADDR(CanvasItemDelegate, revertAndcloseEditor),
                   [&editorReverted](CanvasItemDelegate *) {
                       editorReverted = true;
                   });
    QPoint capturedGridPos(-1, -1);
    stub.set_lamda(ADDR(CanvasViewMenuProxy, showEmptyAreaMenu),
                   [&capturedGridPos](CanvasViewMenuProxy *, const Qt::ItemFlags &, const QPoint gridPos) {
                       capturedGridPos = gridPos;
                   });

    // Act
    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(20, 20), QPoint(120, 120));
    view->contextMenuEvent(&event);

    // Assert
    EXPECT_TRUE(editorReverted);
    EXPECT_EQ(capturedGridPos, view->d->gridAt(QPoint(20, 20)));
}

TEST_F(CanvasViewEventTest, StartDrag_DelayDragEnabled_ReturnsWithoutDrag)
{
    // Arrange
    stub.set_lamda(ADDR(ViewSettingUtil, isDelayDrag),
                   [](const ViewSettingUtil *) -> bool {
                       return true;
                   });

    // Act
    view->startDrag(Qt::CopyAction);

    // Assert
    EXPECT_EQ(hook->startDragCount, 0);
}

TEST_F(CanvasViewEventTest, StartDrag_ExtendHookHandlesDrag_SkipsDefaultDrag)
{
    // Arrange
    stub.set_lamda(ADDR(ViewSettingUtil, isDelayDrag),
                   [](const ViewSettingUtil *) -> bool {
                       return false;
                   });
    stub.set_lamda(VADDR(QAbstractItemView, closePersistentEditor), [](QAbstractItemView *, const QModelIndex &) { __DBG_STUB_INVOKE__; });
    hook->startDragResult = true;

    // Act
    view->startDrag(Qt::CopyAction);

    // Assert
    EXPECT_EQ(hook->startDragCount, 1);
}

TEST_F(CanvasViewEventTest, DragEvents_OperHandlesEvent_BaseBehaviourSkipped)
{
    // Arrange
    bool enterHandled = false;
    bool moveHandled = false;
    bool leaveHandled = false;
    stub.set_lamda(ADDR(DragDropOper, enter),
                   [&enterHandled](DragDropOper *, QDragEnterEvent *) -> bool {
                       enterHandled = true;
                       return true;
                   });
    stub.set_lamda(ADDR(DragDropOper, move),
                   [&moveHandled](DragDropOper *, QDragMoveEvent *) -> bool {
                       moveHandled = true;
                       return true;
                   });
    stub.set_lamda(ADDR(DragDropOper, leave),
                   [&leaveHandled](DragDropOper *, QDragLeaveEvent *) {
                       leaveHandled = true;
                   });

    QMimeData mimeData;
    // Act
    QDragEnterEvent enterEvent(QPoint(10, 10), Qt::CopyAction | Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    view->dragEnterEvent(&enterEvent);
    QDragMoveEvent moveEvent(QPoint(10, 10), Qt::CopyAction | Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    view->dragMoveEvent(&moveEvent);
    QDragLeaveEvent leaveEvent;
    view->dragLeaveEvent(&leaveEvent);

    // Assert
    EXPECT_TRUE(enterHandled);
    EXPECT_TRUE(moveHandled);
    EXPECT_TRUE(leaveHandled);
}

TEST_F(CanvasViewEventTest, DropEvent_OperHandlesDrop_ActivatesWindowAndResetsState)
{
    // Arrange
    QDropEvent *capturedEvent = nullptr;
    stub.set_lamda(ADDR(DragDropOper, drop),
                   [&capturedEvent](DragDropOper *, QDropEvent *event) -> bool {
                       capturedEvent = event;
                       return true;
                   });
    stub.set_lamda(VADDR(QWidget, activateWindow), [](QWidget *) { __DBG_STUB_INVOKE__; });

    // Act
    QMimeData mimeData;
    QDropEvent dropEvent(QPoint(10, 10), Qt::CopyAction | Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    view->dropEvent(&dropEvent);

    // Assert
    EXPECT_EQ(capturedEvent, &dropEvent);
    EXPECT_EQ(view->state(), QAbstractItemView::NoState);
}

TEST_F(CanvasViewEventTest, FocusInEvent_InputMethodEnabled_AttributeEnabled)
{
    // Arrange
    view->d->imEnabled = true;
    view->setAttribute(Qt::WA_InputMethodEnabled, false);
    QFocusEvent event(QEvent::FocusIn);

    // Act
    view->focusInEvent(&event);

    // Assert
    EXPECT_TRUE(view->testAttribute(Qt::WA_InputMethodEnabled));
}

TEST_F(CanvasViewEventTest, FocusOutEvent_AnyFocusOut_StopsDelayDodge)
{
    // Arrange
    bool delayStopped = false;
    bool prepareUpdated = false;
    stub.set_lamda(ADDR(DodgeOper, stopDelayDodge),
                   [&delayStopped](DodgeOper *) {
                       delayStopped = true;
                   });
    stub.set_lamda(ADDR(DodgeOper, updatePrepareDodgeValue),
                   [&prepareUpdated](DodgeOper *, QEvent *) {
                       prepareUpdated = true;
                   });
    QFocusEvent event(QEvent::FocusOut);

    // Act
    view->focusOutEvent(&event);

    // Assert
    EXPECT_TRUE(delayStopped);
    EXPECT_TRUE(prepareUpdated);
}

TEST_F(CanvasViewEventTest, Edit_NoSelection_ReturnsFalse)
{
    // Arrange
    mockModel->appendRow(new QStandardItem("a"));
    EXPECT_EQ(view->selectionModel()->selectedRows().size(), 0);
    QKeyEvent keyEvent(QEvent::MouseButtonPress, Qt::Key_F2, Qt::NoModifier);

    // Act
    bool edited = view->edit(mockModel->index(0, 0), QAbstractItemView::AllEditTriggers, &keyEvent);

    // Assert
    EXPECT_FALSE(edited);
    EXPECT_EQ(view->selectionModel()->selectedRows().size(), 0);
}

TEST_F(CanvasViewEventTest, KeyPressEvent_HookConsumesKey_EventNotPropagated)
{
    // Arrange
    hook->keyPressResult = true;
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);

    // Act
    view->keyPressEvent(&event);

    // Assert
    EXPECT_EQ(hook->keyPressCount, 1);
}

TEST_F(CanvasViewEventTest, MousePressEvent_EmptyAreaLeftButton_StartsBoxSelection)
{
    // Arrange
    hook->mousePressResult = false;
    stub.set_lamda(VADDR(CanvasView, indexAt),
                   [](const CanvasView *, const QPoint &) -> QModelIndex {
                       return QModelIndex();
                   });
    bool touchChecked = false;
    stub.set_lamda(ADDR(ViewSettingUtil, checkTouchDrag),
                   [&touchChecked](ViewSettingUtil *, QMouseEvent *) {
                       touchChecked = true;
                   });
    bool committed = false;
    stub.set_lamda(ADDR(CanvasItemDelegate, commitDataAndCloseEditor),
                   [&committed](CanvasItemDelegate *) {
                       committed = true;
                   });
    bool boxBegin = false;
    stub.set_lamda(ADDR(BoxSelector, beginSelect),
                   [&boxBegin](BoxSelector *, const QPoint &, bool) {
                       boxBegin = true;
                   });
    QModelIndex clickedIndex(0, 0, nullptr, mockModel);
    bool clickCaptured = false;
    stub.set_lamda(ADDR(ClickSelector, click),
                   [&clickCaptured](ClickSelector *, const QModelIndex &) {
                       clickCaptured = true;
                   });

    // Act
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(10, 10),
                      QPointF(110, 110), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    view->mousePressEvent(&event);

    // Assert
    EXPECT_TRUE(touchChecked);
    EXPECT_TRUE(committed);
    EXPECT_TRUE(boxBegin);
    EXPECT_TRUE(clickCaptured);
    EXPECT_EQ(view->state(), QAbstractItemView::DragSelectingState);
}

TEST_F(CanvasViewEventTest, MouseMoveEvent_PlainMove_StateUnchanged)
{
    // Arrange
    EXPECT_EQ(view->state(), QAbstractItemView::NoState);

    // Act
    QMouseEvent event(QEvent::MouseMove, QPointF(50, 50), QPointF(50, 50),
                      QPointF(150, 150), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    view->mouseMoveEvent(&event);

    // Assert
    EXPECT_EQ(view->state(), QAbstractItemView::NoState);
}

TEST_F(CanvasViewEventTest, MouseReleaseEvent_LeftButtonOnEmpty_ForwardsToClickSelector)
{
    // Arrange
    stub.set_lamda(VADDR(CanvasView, indexAt),
                   [](const CanvasView *, const QPoint &) -> QModelIndex {
                       return QModelIndex();
                   });
    QModelIndex released(-1, -1, nullptr, nullptr);
    stub.set_lamda(ADDR(ClickSelector, release),
                   [&released](ClickSelector *, const QModelIndex &index) {
                       released = index;
                   });

    // Act
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10),
                      QPointF(110, 110), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    view->mouseReleaseEvent(&event);

    // Assert
    EXPECT_FALSE(released.isValid());
    EXPECT_EQ(released.model(), nullptr);
}

TEST_F(CanvasViewEventTest, MouseReleaseEvent_RightButton_SkipsClickSelector)
{
    // Arrange
    bool releaseCalled = false;
    stub.set_lamda(ADDR(ClickSelector, release),
                   [&releaseCalled](ClickSelector *, const QModelIndex &) {
                       releaseCalled = true;
                   });

    // Act
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10),
                      QPointF(110, 110), Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    view->mouseReleaseEvent(&event);

    // Assert
    EXPECT_FALSE(releaseCalled);
}

TEST_F(CanvasViewEventTest, WheelEvent_HookConsumsWheel_ChangeIconLevelSkipped)
{
    // Arrange
    hook->wheelResult = true;
    bool iconLevelChanged = false;
    stub.set_lamda(ADDR(CanvasViewMenuProxy, changeIconLevel),
                   [&iconLevelChanged](CanvasViewMenuProxy *, bool) {
                       iconLevelChanged = true;
                   });

    // Act
    QWheelEvent event(QPointF(50, 50), QPointF(150, 150), QPoint(0, 120), QPoint(0, 120),
                      Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    view->wheelEvent(&event);

    // Assert
    EXPECT_EQ(hook->wheelCount, 1);
    EXPECT_FALSE(iconLevelChanged);
}

TEST_F(CanvasViewEventTest, WheelEvent_NoCtrlWheel_DoesNotChangeIconLevel)
{
    // Arrange
    hook->wheelResult = false;
    bool iconLevelChanged = false;
    stub.set_lamda(ADDR(CanvasViewMenuProxy, changeIconLevel),
                   [&iconLevelChanged](CanvasViewMenuProxy *, bool) {
                       iconLevelChanged = true;
                   });

    // Act
    QWheelEvent event(QPointF(50, 50), QPointF(150, 150), QPoint(0, 120), QPoint(0, 120),
                      Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    view->wheelEvent(&event);

    // Assert
    EXPECT_EQ(hook->wheelCount, 1);
    EXPECT_FALSE(iconLevelChanged);
}

TEST_F(CanvasViewEventTest, ItemRect_UnknownItem_ReturnsInvalidRect)
{
    // Arrange
    stub.set_lamda(ADDR(CanvasProxyModel, fileUrl),
                   [](const CanvasProxyModel *, const QModelIndex &) -> QUrl {
                       return QUrl();
                   });

    // Act
    QRect rect = view->itemRect(QModelIndex(0, 0, nullptr, mockModel));

    // Assert
    EXPECT_TRUE(rect.isNull());
    EXPECT_EQ(rect, QRect());
}

TEST_F(CanvasViewEventTest, ItemRect_KnownGridItem_ReturnsGridRect)
{
    // Arrange
    stub.set_lamda(ADDR(CanvasProxyModel, fileUrl),
                   [](const CanvasProxyModel *, const QModelIndex &) -> QUrl {
                       return QUrl("file:///home/a");
                   });
    bool gridPosFound = false;
    stub.set_lamda(ADDR(CanvasViewPrivate, itemGridpos),
                   [&gridPosFound](const CanvasViewPrivate *, const QString &item, QPoint &gridPos) {
                       gridPosFound = item == QString("file:///home/a");
                       gridPos = QPoint(1, 1);
                       return gridPosFound;
                   });
    QRect canned(7, 8, 100, 100);
    stub.set_lamda(ADDR(CanvasViewPrivate, itemRect),
                   [&canned](const CanvasViewPrivate *, const QPoint &) -> QRect {
                       return canned;
                   });

    // Act
    QRect rect = view->itemRect(QModelIndex(0, 0, nullptr, mockModel));

    // Assert
    EXPECT_TRUE(gridPosFound);
    EXPECT_EQ(rect, canned);
}

TEST_F(CanvasViewEventTest, OpenIndexByClicked_MatchingActionWithNullInfo_DoesNotOpen)
{
    // Arrange
    TestProxyModel testProxy;   // flags() reports enabled so the handler reaches openIndex
    stub.set_lamda(ADDR(CanvasView, model),
                   [&testProxy](const CanvasView *) -> CanvasProxyModel * {
                       return &testProxy;
                   });
    stub.set_lamda(ADDR(Application, instance), []() -> Application * {
        return canvas_test::sharedApp();
    });
    stub.set_lamda(ADDR(Application, appAttribute),
                   [](Application::ApplicationAttribute) -> QVariant {
                       return QVariant(static_cast<int>(CanvasViewPrivate::ClickedAction::kClicked));
                   });
    stub.set_lamda(ADDR(WindowUtils, keyCtrlIsPressed),
                   []() -> bool {
                       return false;
                   });
    stub.set_lamda(ADDR(WindowUtils, keyShiftIsPressed),
                   []() -> bool {
                       return false;
                   });
    stub.set_lamda(ADDR(CanvasProxyModel, fileInfo),
                   [](const CanvasProxyModel *, const QModelIndex &) -> FileInfoPointer {
                       return FileInfoPointer();
                   });
    bool openFilesCalled = false;
    stub.set_lamda(static_cast<void (FileOperatorProxy::*)(const CanvasView *, const QList<QUrl> &)>(&FileOperatorProxy::openFiles),
                   [&openFilesCalled](FileOperatorProxy *, const CanvasView *, const QList<QUrl> &) {
                       openFilesCalled = true;
                   });

    // Act
    view->d->openIndexByClicked(CanvasViewPrivate::ClickedAction::kClicked, QModelIndex(0, 0, nullptr, mockModel));

    // Assert: flags on an empty proxy model lack ItemIsEnabled, so no open happens.
    EXPECT_FALSE(openFilesCalled);
}

TEST_F(CanvasViewEventTest, OpenIndex_NullFileInfo_DoesNotOpenFiles)
{
    // Arrange
    stub.set_lamda(ADDR(CanvasProxyModel, fileInfo),
                   [](const CanvasProxyModel *, const QModelIndex &) -> FileInfoPointer {
                       return FileInfoPointer();
                   });
    bool openFilesCalled = false;
    stub.set_lamda(static_cast<void (FileOperatorProxy::*)(const CanvasView *, const QList<QUrl> &)>(&FileOperatorProxy::openFiles),
                   [&openFilesCalled](FileOperatorProxy *, const CanvasView *, const QList<QUrl> &) {
                       openFilesCalled = true;
                   });

    // Act
    view->d->openIndex(QModelIndex(0, 0, nullptr, mockModel));

    // Assert
    EXPECT_FALSE(openFilesCalled);
}

TEST_F(CanvasViewEventTest, OpenIndexByClicked_NonMatchingAction_SkipsOpen)
{
    // Arrange
    stub.set_lamda(ADDR(Application, instance), []() -> Application * {
        return canvas_test::sharedApp();
    });
    stub.set_lamda(ADDR(Application, appAttribute),
                   [](Application::ApplicationAttribute) -> QVariant {
                       return QVariant(static_cast<int>(CanvasViewPrivate::ClickedAction::kDoubleClicked));
                   });
    bool openIndexCalled = false;
    stub.set_lamda(ADDR(CanvasViewPrivate, openIndex),
                   [&openIndexCalled](CanvasViewPrivate *, const QModelIndex &) {
                       openIndexCalled = true;
                   });

    // Act
    view->d->openIndexByClicked(CanvasViewPrivate::ClickedAction::kClicked, QModelIndex(0, 0, nullptr, mockModel));

    // Assert
    EXPECT_FALSE(openIndexCalled);
}

TEST_F(CanvasViewEventTest, FindIndex_EmptyGrid_ReturnsInvalidIndex)
{
    // Arrange
    stub.set_lamda(ADDR(CanvasGrid, item),
                   [](const CanvasGrid *, int, const QPoint &) -> QString {
                       return QString();
                   });
    stub.set_lamda(static_cast<QModelIndex (CanvasProxyModel::*)(const QUrl &, int) const>(&CanvasProxyModel::index),
                   [](const CanvasProxyModel *, const QUrl &, int) -> QModelIndex {
                       return QModelIndex();
                   });

    // Act
    QModelIndex found = view->d->findIndex(QString("a"), true, QModelIndex(), false, false);

    // Assert
    EXPECT_FALSE(found.isValid());
    EXPECT_EQ(found, QModelIndex());
}

TEST_F(CanvasViewEventTest, FindIndex_MatchingPinyinName_ReturnsModelIndex)
{
    // Arrange
    TestProxyModel testProxy;   // data() reports a matching pinyin name
    stub.set_lamda(ADDR(CanvasView, model),
                   [&testProxy](const CanvasView *) -> CanvasProxyModel * {
                       return &testProxy;
                   });
    QModelIndex canned(0, 0, nullptr, mockModel);
    stub.set_lamda(ADDR(CanvasViewPrivate, visualItem),
                   [](const CanvasViewPrivate *, const QPoint &) -> QString {
                       return QString("file:///home/a");
                   });
    stub.set_lamda(static_cast<QModelIndex (CanvasProxyModel::*)(const QUrl &, int) const>(&CanvasProxyModel::index),
                   [&canned](const CanvasProxyModel *, const QUrl &, int) -> QModelIndex {
                       return canned;
                   });

    // Act
    QModelIndex found = view->d->findIndex(QString("app"), true, QModelIndex(), false, false);

    // Assert
    EXPECT_TRUE(found.isValid());
    EXPECT_EQ(found, canned);
}

TEST_F(CanvasViewEventTest, LastIndex_WithOverloadItem_ReturnsOverloadModelIndex)
{
    // Arrange
    QModelIndex canned(0, 0, nullptr, mockModel);
    QUrl capturedUrl;
    stub.set_lamda(ADDR(CanvasGrid, overloadItems),
                   [](const CanvasGrid *, int) -> QStringList {
                       return { QString("file:///home/overload") };
                   });
    stub.set_lamda(static_cast<QModelIndex (CanvasProxyModel::*)(const QUrl &, int) const>(&CanvasProxyModel::index),
                   [&canned, &capturedUrl](const CanvasProxyModel *, const QUrl &url, int) -> QModelIndex {
                       capturedUrl = url;
                       return canned;
                   });

    // Act
    QModelIndex last = view->d->lastIndex();

    // Assert
    EXPECT_EQ(capturedUrl, QUrl("file:///home/overload"));
    EXPECT_TRUE(last.isValid());
    EXPECT_EQ(last, canned);
}

TEST_F(CanvasViewEventTest, LastIndex_EmptyGrid_ReturnsInvalidIndex)
{
    // Arrange
    stub.set_lamda(ADDR(CanvasGrid, overloadItems),
                   [](const CanvasGrid *, int) -> QStringList {
                       return QStringList();
                   });
    stub.set_lamda(ADDR(CanvasGrid, item),
                   [](const CanvasGrid *, int, const QPoint &) -> QString {
                       return QString();
                   });
    stub.set_lamda(static_cast<QModelIndex (CanvasProxyModel::*)(const QUrl &, int) const>(&CanvasProxyModel::index),
                   [](const CanvasProxyModel *, const QUrl &, int) -> QModelIndex {
                       return QModelIndex();
                   });

    // Act
    QModelIndex last = view->d->lastIndex();

    // Assert
    EXPECT_FALSE(last.isValid());
    EXPECT_EQ(last, QModelIndex());
}

TEST_F(CanvasViewEventTest, IndexAt_PointInsideIconRect_ReturnsItemIndex)
{
    // Arrange
    stub.set_lamda(ADDR(CanvasItemDelegate, mayExpand),
                   [](const CanvasItemDelegate *, QModelIndex *) -> bool {
                       return false;
                   });
    stub.set_lamda(ADDR(CanvasViewPrivate, visualItem),
                   [](const CanvasViewPrivate *, const QPoint &) -> QString {
                       return QString("file:///home/a");
                   });
    QModelIndex canned(0, 0, nullptr, mockModel);
    stub.set_lamda(static_cast<QModelIndex (CanvasProxyModel::*)(const QUrl &, int) const>(&CanvasProxyModel::index),
                   [&canned](const CanvasProxyModel *, const QUrl &, int) -> QModelIndex {
                       return canned;
                   });
    stub.set_lamda(ADDR(CanvasView, itemPaintGeomertys),
                   [](const CanvasView *, const QModelIndex &) -> QList<QRect> {
                       return { QRect(0, 0, 100, 100), QRect(0, 100, 100, 40) };
                   });

    // Act
    QModelIndex at = view->indexAt(QPoint(50, 50));

    // Assert
    EXPECT_TRUE(at.isValid());
    EXPECT_EQ(at, canned);
}

TEST_F(CanvasViewEventTest, IndexAt_ExpandedItemInsideIconRect_ReturnsCurrentIndex)
{
    // Arrange: expanded item branch exercises indexAt's own checkRect lambda.
    QModelIndex current(0, 0, nullptr, mockModel);
    stub.set_lamda(VADDR(QAbstractItemView, currentIndex),
                   [&current](const QAbstractItemView *) -> QModelIndex {
                       return current;
                   });
    stub.set_lamda(VADDR(QAbstractItemView, isPersistentEditorOpen),
                   [](const QAbstractItemView *, const QModelIndex &) -> bool {
                       return false;
                   });
    stub.set_lamda(ADDR(CanvasItemDelegate, mayExpand),
                   [](const CanvasItemDelegate *, QModelIndex *) -> bool {
                       return true;
                   });
    stub.set_lamda(ADDR(CanvasView, itemPaintGeomertys),
                   [](const CanvasView *, const QModelIndex &) -> QList<QRect> {
                       return { QRect(0, 0, 100, 100), QRect(0, 100, 100, 40) };
                   });

    // Act
    QModelIndex at = view->indexAt(QPoint(50, 50));

    // Assert
    EXPECT_TRUE(at.isValid());
    EXPECT_EQ(at, current);
}

TEST_F(CanvasViewEventTest, IndexAt_PointOutsideAllRects_ReturnsInvalidIndex)
{
    // Arrange
    stub.set_lamda(ADDR(CanvasItemDelegate, mayExpand),
                   [](const CanvasItemDelegate *, QModelIndex *) -> bool {
                       return false;
                   });
    stub.set_lamda(ADDR(CanvasViewPrivate, visualItem),
                   [](const CanvasViewPrivate *, const QPoint &) -> QString {
                       return QString("file:///home/a");
                   });
    QModelIndex canned(0, 0, nullptr, mockModel);
    stub.set_lamda(static_cast<QModelIndex (CanvasProxyModel::*)(const QUrl &, int) const>(&CanvasProxyModel::index),
                   [&canned](const CanvasProxyModel *, const QUrl &, int) -> QModelIndex {
                       return canned;
                   });
    stub.set_lamda(ADDR(CanvasView, itemPaintGeomertys),
                   [](const CanvasView *, const QModelIndex &) -> QList<QRect> {
                       return { QRect(0, 0, 10, 10) };
                   });

    // Act
    QModelIndex at = view->indexAt(QPoint(500, 500));

    // Assert
    EXPECT_FALSE(at.isValid());
    EXPECT_NE(at, canned);
}
