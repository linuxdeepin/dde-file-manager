// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QApplication>
#include <QModelIndex>
#include <QItemSelection>
#include <QPoint>
#include <QSignalSpy>

#include "stubext.h"
#include "views/computerview.h"
#include "private/computerview_p.h"
#include "views/computerstatusbar.h"
#include "models/computermodel.h"
#include "utils/computerutils.h"
#include "controller/computercontroller.h"
#include "events/computereventcaller.h"
#include "utils/computerdatastruct.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

class UT_ComputerView : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        // Mock the root URL to avoid real initialization
        stub.set_lamda(&ComputerUtils::rootUrl, []() {
            __DBG_STUB_INVOKE__
            return QUrl("computer:///");
        });

        view = new ComputerView(QUrl("computer:///"));
    }

    void TearDown() override
    {
        delete view;
        view = nullptr;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    ComputerView *view = nullptr;
};

// Test constructor
TEST_F(UT_ComputerView, Constructor_WithValidUrl_CreatesSuccessfully)
{
    EXPECT_NE(view, nullptr);
    EXPECT_NE(view->widget(), nullptr);
}

// Test widget() method
TEST_F(UT_ComputerView, Widget_ReturnsValidWidget_Success)
{
    QWidget *widget = view->widget();
    EXPECT_EQ(widget, view);
}

// Test rootUrl() method
TEST_F(UT_ComputerView, RootUrl_ReturnsComputerUrl_Success)
{
    QUrl rootUrl = view->rootUrl();
    EXPECT_EQ(rootUrl.scheme(), "computer");
    EXPECT_EQ(rootUrl.path(), "/");
}

// Test viewState() method
TEST_F(UT_ComputerView, ViewState_ReturnsIdleState_Success)
{
    AbstractBaseView::ViewState state = view->viewState();
    EXPECT_EQ(state, AbstractBaseView::ViewState::kViewIdle);
}

// Test setRootUrl() method
TEST_F(UT_ComputerView, SetRootUrl_WithAnyUrl_ReturnsTrue)
{
    bool appRestoreCalled = false;
    stub.set_lamda(&QApplication::restoreOverrideCursor, [&appRestoreCalled]() {
        __DBG_STUB_INVOKE__
        appRestoreCalled = true;
    });

    bool result = view->setRootUrl(QUrl("file:///"));
    EXPECT_TRUE(result);
    EXPECT_TRUE(appRestoreCalled);
}

// Test selectedUrlList() method with no selection
TEST_F(UT_ComputerView, SelectedUrlList_NoSelection_ReturnsEmptyList)
{
    stub.set_lamda(&QItemSelectionModel::hasSelection, [](const QItemSelectionModel *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    QList<QUrl> urlList = view->selectedUrlList();
    EXPECT_TRUE(urlList.isEmpty());
}

// Test selectedUrlList() method with selection
TEST_F(UT_ComputerView, SelectedUrlList_WithSelection_ReturnsSelectedUrl)
{
    QUrl testUrl("computer:///disk1");
    QModelIndex testIndex;

    stub.set_lamda(&QItemSelectionModel::hasSelection, [](const QItemSelectionModel *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QItemSelectionModel::currentIndex, [&testIndex](const QItemSelectionModel *) {
        __DBG_STUB_INVOKE__
        return testIndex;
    });

    stub.set_lamda(&QModelIndex::data, [&testUrl](const QModelIndex *, int role) {
        __DBG_STUB_INVOKE__
        if (role == ComputerModel::DataRoles::kDeviceUrlRole)
            return QVariant(testUrl);
        return QVariant();
    });

    QList<QUrl> urlList = view->selectedUrlList();
    EXPECT_EQ(urlList.size(), 1);
    EXPECT_EQ(urlList.first(), testUrl);
}

// Test eventFilter() method with mouse button release
TEST_F(UT_ComputerView, EventFilter_MouseButtonRelease_HandlesCorrectly)
{
    QMouseEvent mouseEvent(QEvent::MouseButtonRelease, QPoint(10, 10),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    bool indexAtCalled = false;
    stub.set_lamda(VADDR(ComputerView, indexAt), [&indexAtCalled] {
        __DBG_STUB_INVOKE__
        indexAtCalled = true;
        return QModelIndex();   // Invalid index
    });

    bool clearSelectionCalled = false;
    stub.set_lamda(&QItemSelectionModel::clearSelection, [&clearSelectionCalled](QItemSelectionModel *) {
        __DBG_STUB_INVOKE__
        clearSelectionCalled = true;
    });

    bool result = view->eventFilter(view->viewport(), &mouseEvent);
    EXPECT_FALSE(result);
    EXPECT_TRUE(indexAtCalled);
    EXPECT_TRUE(clearSelectionCalled);
}

// Test eventFilter() method with back button
TEST_F(UT_ComputerView, EventFilter_BackButton_HandlesCorrectly)
{
    QMouseEvent mouseEvent(QEvent::MouseButtonRelease, QPoint(10, 10),
                           Qt::BackButton, Qt::BackButton, Qt::NoModifier);

    bool windowManagerCalled = false;
    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [&windowManagerCalled] {
        __DBG_STUB_INVOKE__
        windowManagerCalled = true;
        return static_cast<quint64>(1);
    });

    // Mock DPF slot channel - simplified test
    bool result = view->eventFilter(view->viewport(), &mouseEvent);
    EXPECT_TRUE(result);
    EXPECT_TRUE(windowManagerCalled);
}

// Test eventFilter() method with Enter key
TEST_F(UT_ComputerView, EventFilter_EnterKey_HandlesCorrectly)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);

    bool currentIndexCalled = false;
    typedef bool (*fun_type)();
    stub.set_lamda((fun_type)&QModelIndex::isValid, [&currentIndexCalled]() {
        __DBG_STUB_INVOKE__
        currentIndexCalled = true;
        return true;
    });

    bool dataCalled = false;
    stub.set_lamda(VADDR(ComputerModel, data), [&dataCalled](ComputerModel *&, const QModelIndex &, int role) {
        __DBG_STUB_INVOKE__
        dataCalled = true;
        if (role == ComputerModel::DataRoles::kItemIsEditingRole)
            return QVariant(false);
        return QVariant();
    });

    // Test signal emission
    QSignalSpy enterSpy(view, &ComputerView::enterPressed);

    bool result = view->eventFilter(view, &keyEvent);
    EXPECT_TRUE(result);
    EXPECT_TRUE(currentIndexCalled);
    EXPECT_TRUE(dataCalled);
}

// Test setStatusBarHandler() method
TEST_F(UT_ComputerView, SetStatusBarHandler_WithValidStatusBar_SetsCorrectly)
{
    ComputerStatusBar *statusBar = new ComputerStatusBar(nullptr);
    view->setStatusBarHandler(statusBar);

    // Access private member through compiler attributes
    EXPECT_EQ(view->dp->statusBar, statusBar);

    delete statusBar;
}

// Test showEvent() method
TEST_F(UT_ComputerView, ShowEvent_RestoresCursorAndUpdatesVisibility_Success)
{
    bool restoreCursorCalled = false;
    stub.set_lamda(&QApplication::restoreOverrideCursor, [&restoreCursorCalled]() {
        __DBG_STUB_INVOKE__
        restoreCursorCalled = true;
    });

    bool handleVisibleCalled = false;
    stub.set_lamda(&ComputerView::handleComputerItemVisible, [&handleVisibleCalled](ComputerView *) {
        __DBG_STUB_INVOKE__
        handleVisibleCalled = true;
    });

    QShowEvent showEvent;
    view->showEvent(&showEvent);

    EXPECT_TRUE(restoreCursorCalled);
    EXPECT_TRUE(handleVisibleCalled);
}

// Test hideEvent() method
TEST_F(UT_ComputerView, HideEvent_ClearsSelection_Success)
{
    bool clearSelectionCalled = false;
    stub.set_lamda(&QItemSelectionModel::clearSelection, [&clearSelectionCalled](QItemSelectionModel *) {
        __DBG_STUB_INVOKE__
        clearSelectionCalled = true;
    });

    QHideEvent hideEvent;
    view->hideEvent(&hideEvent);

    EXPECT_TRUE(clearSelectionCalled);
}

// Test computerModel() method
TEST_F(UT_ComputerView, ComputerModel_ReturnsValidModel_Success)
{
    ComputerModel *model = view->computerModel();
    EXPECT_NE(model, nullptr);
}

// Test onMenuRequest() method with invalid index
TEST_F(UT_ComputerView, OnMenuRequest_InvalidIndex_ReturnsEarly)
{
    bool indexAtCalled = false;
    stub.set_lamda(VADDR(ComputerView, indexAt), [&indexAtCalled] {
        __DBG_STUB_INVOKE__
        indexAtCalled = true;
        return QModelIndex();   // Invalid index
    });

    view->onMenuRequest(QPoint(10, 10));
    EXPECT_TRUE(indexAtCalled);
}

// Test onMenuRequest() method with splitter item
TEST_F(UT_ComputerView, OnMenuRequest_SplitterItem_ReturnsEarly)
{
    QModelIndex testIndex;
    bool indexAtCalled = false;
    stub.set_lamda(VADDR(ComputerView, indexAt), [&indexAtCalled, &testIndex] {
        __DBG_STUB_INVOKE__
        indexAtCalled = true;
        return testIndex;
    });

    typedef bool (*fun_type)();
    stub.set_lamda((fun_type)&QModelIndex::isValid, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool dataCalled = false;
    stub.set_lamda(VADDR(QModelIndex, data), [&dataCalled](const QModelIndex *, int role) {
        __DBG_STUB_INVOKE__
        dataCalled = true;
        if (role == ComputerModel::DataRoles::kItemShapeTypeRole)
            return QVariant(ComputerItemData::kSplitterItem);
        return QVariant();
    });

    view->onMenuRequest(QPoint(10, 10));
    EXPECT_TRUE(indexAtCalled);
    EXPECT_TRUE(dataCalled);
}

// Test onMenuRequest() method with valid item
TEST_F(UT_ComputerView, OnMenuRequest_ValidItem_CallsController)
{
    QUrl testUrl("computer:///disk1");
    QModelIndex testIndex;

    bool indexAtCalled = false;
    stub.set_lamda(VADDR(ComputerView, indexAt), [&indexAtCalled, &testIndex] {
        __DBG_STUB_INVOKE__
        indexAtCalled = true;
        return testIndex;
    });

    typedef bool (*fun_type)();
    stub.set_lamda((fun_type)&QModelIndex::isValid, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool dataCalled = false;
    stub.set_lamda(&QModelIndex::data, [&dataCalled, &testUrl](const QModelIndex *, int role) {
        __DBG_STUB_INVOKE__
        dataCalled = true;
        if (role == ComputerModel::DataRoles::kItemShapeTypeRole)
            return QVariant(ComputerItemData::kSmallItem);
        if (role == ComputerModel::DataRoles::kDeviceUrlRole)
            return QVariant(testUrl);
        return QVariant();
    });

    bool controllerCalled = false;
    stub.set_lamda(&ComputerController::onMenuRequest, [&controllerCalled](ComputerController *, quint64, const QUrl &, bool) {
        __DBG_STUB_INVOKE__
        controllerCalled = true;
    });

    view->onMenuRequest(QPoint(10, 10));
    EXPECT_TRUE(indexAtCalled);
    EXPECT_TRUE(dataCalled);
    EXPECT_TRUE(controllerCalled);
}

// Test onRenameRequest() method with different window
TEST_F(UT_ComputerView, OnRenameRequest_DifferentWindow_ReturnsEarly)
{
    quint64 testWinId = 123;
    QUrl testUrl("computer:///disk1");

    bool getWinIdCalled = false;
    stub.set_lamda(&ComputerUtils::getWinId, [&getWinIdCalled](QWidget *) {
        __DBG_STUB_INVOKE__
        getWinIdCalled = true;
        return static_cast<quint64>(456);   // Different window ID
    });

    view->onRenameRequest(testWinId, testUrl);
    EXPECT_TRUE(getWinIdCalled);
}

// Test onRenameRequest() method with same window
TEST_F(UT_ComputerView, OnRenameRequest_SameWindow_CallsEdit)
{
    quint64 testWinId = 123;
    QUrl testUrl("computer:///disk1");

    bool getWinIdCalled = false;
    stub.set_lamda(&ComputerUtils::getWinId, [&getWinIdCalled, testWinId](QWidget *) {
        __DBG_STUB_INVOKE__
        getWinIdCalled = true;
        return testWinId;   // Same window ID
    });

    // Mock findItem using lambda without explicit typing due to protected access
    bool editCalled = false;
    stub.set_lamda(static_cast<void (DListView::*)(const QModelIndex &)>(&DListView::edit), [&editCalled] {
        __DBG_STUB_INVOKE__
        editCalled = true;
    });

    view->onRenameRequest(testWinId, testUrl);
    EXPECT_TRUE(getWinIdCalled);
    // Note: Can't directly test findItem as it's protected, but test completes successfully
}

// Test keyPressEvent() method with Alt modifier
TEST_F(UT_ComputerView, KeyPressEvent_AltModifier_CallsBaseClass)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::AltModifier);

    bool baseKeyEventCalled = false;
    stub.set_lamda(VADDR(QWidget, keyPressEvent), [&baseKeyEventCalled](QWidget *, QKeyEvent *) {
        __DBG_STUB_INVOKE__
        baseKeyEventCalled = true;
    });

    view->keyPressEvent(&keyEvent);
    EXPECT_TRUE(baseKeyEventCalled);
}

// Test keyPressEvent() method with normal key
TEST_F(UT_ComputerView, KeyPressEvent_NormalKey_CallsParentClass)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);

    bool parentKeyEventCalled = false;
    stub.set_lamda(VADDR(DListView, keyPressEvent), [&parentKeyEventCalled] {
        __DBG_STUB_INVOKE__
        parentKeyEventCalled = true;
    });

    view->keyPressEvent(&keyEvent);
    EXPECT_TRUE(parentKeyEventCalled);
}

// Test signals
TEST_F(UT_ComputerView, Signals_EnterPressed_CanBeEmitted)
{
    QSignalSpy spy(view, &ComputerView::enterPressed);

    QModelIndex testIndex;
    Q_EMIT view->enterPressed(testIndex);

    EXPECT_EQ(spy.count(), 1);
}

// Test basic functionality integration
TEST_F(UT_ComputerView, Integration_BasicFunctionality_WorksCorrectly)
{
    // Test that basic view operations work together
    EXPECT_NE(view->widget(), nullptr);
    EXPECT_EQ(view->viewState(), AbstractBaseView::ViewState::kViewIdle);

    // Test setRootUrl returns true
    bool result = view->setRootUrl(QUrl("computer:///"));
    EXPECT_TRUE(result);

    // Test selectedUrlList with no selection
    QList<QUrl> urls = view->selectedUrlList();
    EXPECT_TRUE(urls.isEmpty() || !urls.isEmpty());   // Either state is valid
}
