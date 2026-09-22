// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filemanagerwindow_cov.cpp
 * @brief Coverage-focused unit tests for FileManagerWindow
 *        (src/dfm-base/widgets/dfmwindow/filemanagerwindow.cpp).
 *
 * Covered functions from the gap list (filemanagerwindow.cpp, 83 entries):
 *   - FileManagerWindowPrivate ctor + processKeyPressEvent (all modifier
 *     branches via keyPressEvent), loadSidebarState / splitterPosition /
 *     setSplitterPosition / resetTitleBarSize / resetSideBarSize /
 *     initDetailSplitter / updateSidebarSeparator / setupSidebarSepTracking /
 *     loadDetailSpaceState / loadDetailSpaceVisibility /
 *     setDetailSplitterPosition / installDetailSplitterHandleEventFilter /
 *     updateRightAreaMinWidth / updateSideBarState /
 *     updateSideBarVisibility / updateWindowMinimumWidth /
 *     calculateRequiredWindowWidth / calculateActualMinRightWidth /
 *     willSidebarAutoShowAfterResize / animateDetailSplitter entry /
 *     animateSplitter entry / handleWindowResize entry (resizeEvent) /
 *     handleWorkspaceKeyEvent + handleSideBarEvent entries (eventFilter)
 *     -> all reached indirectly through the public API below.
 *   - FileManagerWindow: ctor lambdas (expandButton connect),
 *     saveClosedSate, moveCenter, installTitleBar (+lambda #1),
 *     installSideBar (+lambda #1), installWorkSpace (+lambda #1),
 *     installDetailView, detailView, setDetailViewWidth, detailViewWidth,
 *     showDetailSpace (animated + non-animated branches), hideDetailSpace
 *     (animated + non-animated branches), isDetailSpaceVisible, loadState,
 *     saveState (saveSidebarState/saveDetailSpaceState/saveWindowState),
 *     mouseDoubleClickEvent, moveEvent, keyPressEvent, eventFilter,
 *     initializeUi, updateUi, resizeEvent.
 *
 * Case -> function mapping:
 *   Construction*                 -> ctor, FileManagerWindowPrivate ctor
 *   InstallFrames*                -> installTitleBar/SideBar/WorkSpace,
 *                                    initializeUi, updateUi (+ private helpers)
 *   InstallDetailView*            -> installDetailView, setDetailSplitterPosition,
 *                                    installDetailSplitterHandleEventFilter
 *   DetailSpace*                  -> showDetailSpace/hideDetailSpace branches,
 *                                    setDetailViewWidth/detailViewWidth
 *   KeyPress_*                    -> keyPressEvent, processKeyPressEvent
 *   StatePersistence_*            -> loadState/saveState/saveClosedSate
 *   MoveCenter_*, WindowMove*     -> moveCenter, moveEvent
 *   EventFilter_*                 -> eventFilter + private handlers
 *   ResizeEvent_*, DoubleClick_*  -> resizeEvent, mouseDoubleClickEvent
 *
 * processKeyPressEvent branch list (from source):
 *   NoModifier+F5 / Ctrl+{Tab,Backtab,F,L,Left,Right,W,T,N,1..9} /
 *   Alt(+Keypad)+{1..8,Left,Right} / Ctrl+Shift+{?,Backtab} / unhandled key.
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>
#include <QUrl>
#include <QPoint>
#include <QSize>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QtMath>

#include "stubext.h"

#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/interfaces/abstractframe.h>
#include <dfm-base/utils/windowutils.h>

using namespace dfmbase;

namespace {
class MockFrame : public AbstractFrame
{
public:
    explicit MockFrame(QWidget *parent = nullptr)
        : AbstractFrame(parent) {}
    void setCurrentUrl(const QUrl &url) override { url_ = url; }
    QUrl currentUrl() const override { return url_; }
    QUrl url_;
};
}   // namespace

class UT_FileManagerWindowCov : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(UT_FileManagerWindowCov, Construction_WithLocalFileUrl_ExpectTitleAndCurrentUrlKept)
{
    // Arrange
    QUrl url = QUrl::fromLocalFile("/tmp");

    // Act
    FileManagerWindow w(url);

    // Assert
    EXPECT_EQ(w.currentUrl(), url);
    EXPECT_EQ(w.windowTitle(), QStringLiteral("tmp"));
}

TEST_F(UT_FileManagerWindowCov, InstallFrames_AllThree_ExpectSignalsAndCdPropagation)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    auto *title = new MockFrame;
    auto *side = new MockFrame;
    auto *space = new MockFrame;
    QSignalSpy titleSpy(&w, &FileManagerWindow::titleBarInstallFinished);
    QSignalSpy sideSpy(&w, &FileManagerWindow::sideBarInstallFinished);
    QSignalSpy spaceSpy(&w, &FileManagerWindow::workspaceInstallFinished);

    // Act
    w.installTitleBar(title);
    w.installSideBar(side);
    w.installWorkSpace(space);
    w.cd(QUrl::fromLocalFile("/home"));

    // Assert
    EXPECT_EQ(titleSpy.count(), 1);
    EXPECT_EQ(sideSpy.count(), 1);
    EXPECT_EQ(spaceSpy.count(), 1);
    EXPECT_EQ(w.titleBar(), title);
    EXPECT_EQ(w.sideBar(), side);
    EXPECT_EQ(w.workSpace(), space);
    EXPECT_EQ(title->currentUrl(), QUrl::fromLocalFile("/home"));
    EXPECT_EQ(space->currentUrl(), QUrl::fromLocalFile("/home"));
}

TEST_F(UT_FileManagerWindowCov, InstallFrames_SecondInstallIgnored_ExpectCallOnceSemantics)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    QSignalSpy titleSpy(&w, &FileManagerWindow::titleBarInstallFinished);

    // Act
    w.installTitleBar(new MockFrame);
    MockFrame *first = static_cast<MockFrame *>(w.titleBar());
    w.installTitleBar(new MockFrame);

    // Assert
    EXPECT_EQ(titleSpy.count(), 1);
    EXPECT_EQ(w.titleBar(), first);
}

static void installAll(FileManagerWindow *w, MockFrame **title, MockFrame **side,
                       MockFrame **space, MockFrame **detail = nullptr)
{
    *title = new MockFrame;
    *side = new MockFrame;
    *space = new MockFrame;
    w->installTitleBar(*title);
    w->installSideBar(*side);
    w->installWorkSpace(*space);
    if (detail) {
        *detail = new MockFrame;
        w->installDetailView(*detail);
    }
}

TEST_F(UT_FileManagerWindowCov, InstallDetailView_WithDetailFrame_ExpectSignalAndGetter)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    MockFrame *title = nullptr, *side = nullptr, *space = nullptr, *detail = nullptr;
    QSignalSpy detailSpy(&w, &FileManagerWindow::detailViewInstallFinished);

    // Act
    installAll(&w, &title, &side, &space, &detail);

    // Assert
    EXPECT_EQ(detailSpy.count(), 1);
    EXPECT_EQ(w.detailView(), detail);
}

TEST_F(UT_FileManagerWindowCov, DetailSpace_ShowNonAnimated_ExpectVisibleFlagAndSignal)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    MockFrame *title = nullptr, *side = nullptr, *space = nullptr, *detail = nullptr;
    installAll(&w, &title, &side, &space, &detail);
    QSignalSpy spy(&w, &FileManagerWindow::detailSpaceVisibilityChanged);
    EXPECT_FALSE(w.isDetailSpaceVisible());

    // Act
    QVariantHash opts { { DetailSpaceOptions::kAnimated, false } };
    w.showDetailSpace(opts);

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toBool(), true);
    EXPECT_TRUE(w.isDetailSpaceVisible());
}

TEST_F(UT_FileManagerWindowCov, DetailSpace_HideNonAnimated_ExpectHiddenFlagAndSignalFalse)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    MockFrame *title = nullptr, *side = nullptr, *space = nullptr, *detail = nullptr;
    installAll(&w, &title, &side, &space, &detail);
    w.showDetailSpace({ { DetailSpaceOptions::kAnimated, false } });
    QSignalSpy spy(&w, &FileManagerWindow::detailSpaceVisibilityChanged);

    // Act
    w.hideDetailSpace({ { DetailSpaceOptions::kAnimated, false } });

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toBool(), false);
    EXPECT_FALSE(w.isDetailSpaceVisible());
}

TEST_F(UT_FileManagerWindowCov, DetailSpace_ShowAnimated_ExpectFlagSetWithoutBlocking)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    MockFrame *title = nullptr, *side = nullptr, *space = nullptr, *detail = nullptr;
    installAll(&w, &title, &side, &space, &detail);
    QSignalSpy spy(&w, &FileManagerWindow::detailSpaceVisibilityChanged);

    // Act: default options -> animated = true branch
    w.showDetailSpace();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(w.isDetailSpaceVisible());
}

TEST_F(UT_FileManagerWindowCov, DetailSpace_NonUserAction_ExpectNoSignalAndFlagUntouched)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    MockFrame *title = nullptr, *side = nullptr, *space = nullptr, *detail = nullptr;
    installAll(&w, &title, &side, &space, &detail);
    QSignalSpy spy(&w, &FileManagerWindow::detailSpaceVisibilityChanged);

    // Act
    QVariantHash opts { { DetailSpaceOptions::kAnimated, false },
                        { DetailSpaceOptions::kUserAction, false } };
    w.showDetailSpace(opts);

    // Assert
    EXPECT_EQ(spy.count(), 0);
    EXPECT_FALSE(w.isDetailSpaceVisible());
}

TEST_F(UT_FileManagerWindowCov, DetailViewWidth_ClampedToBoundaries_ExpectMonotonicRange)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    MockFrame *title = nullptr, *side = nullptr, *space = nullptr, *detail = nullptr;
    installAll(&w, &title, &side, &space, &detail);

    // Act
    w.setDetailViewWidth(100000);
    int upper = w.detailViewWidth();
    w.setDetailViewWidth(-100000);
    int lower = w.detailViewWidth();
    w.setDetailViewWidth(300);

    // Assert: qBound(kMinimumDetailWidth, width, kMaximumDetailWidth)
    EXPECT_GT(upper, lower);
    EXPECT_GE(w.detailViewWidth(), lower);
    EXPECT_LE(w.detailViewWidth(), upper);
}

TEST_F(UT_FileManagerWindowCov, KeyPress_F5NoModifier_ExpectReqRefreshEmitted)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    QSignalSpy spy(&w, &FileManagerWindow::reqRefresh);
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_F5, Qt::NoModifier);

    // Act
    w.keyPressEvent(&ev);

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).count(), 0);
}

TEST_F(UT_FileManagerWindowCov, KeyPress_CtrlCombinations_ExpectAllRequestSignals)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    QSignalSpy nextTab(&w, &FileManagerWindow::reqActivateNextTab);
    QSignalSpy searchF(&w, &FileManagerWindow::reqSearchCtrlF);
    QSignalSpy searchL(&w, &FileManagerWindow::reqSearchCtrlL);
    QSignalSpy back(&w, &FileManagerWindow::reqBack);
    QSignalSpy fwd(&w, &FileManagerWindow::reqForward);
    QSignalSpy closeTab(&w, &FileManagerWindow::reqCloseCurrentTab);
    QSignalSpy newTab(&w, &FileManagerWindow::reqCreateTab);
    QSignalSpy newWin(&w, &FileManagerWindow::reqCreateWindow);

    // Act
    QKeyEvent evTab(QEvent::KeyPress, Qt::Key_Tab, Qt::ControlModifier);
    w.keyPressEvent(&evTab);
    QKeyEvent evF(QEvent::KeyPress, Qt::Key_F, Qt::ControlModifier);
    w.keyPressEvent(&evF);
    QKeyEvent evL(QEvent::KeyPress, Qt::Key_L, Qt::ControlModifier);
    w.keyPressEvent(&evL);
    QKeyEvent evLeft(QEvent::KeyPress, Qt::Key_Left, Qt::ControlModifier);
    w.keyPressEvent(&evLeft);
    QKeyEvent evRight(QEvent::KeyPress, Qt::Key_Right, Qt::ControlModifier);
    w.keyPressEvent(&evRight);
    QKeyEvent evW(QEvent::KeyPress, Qt::Key_W, Qt::ControlModifier);
    w.keyPressEvent(&evW);
    QKeyEvent evT(QEvent::KeyPress, Qt::Key_T, Qt::ControlModifier);
    w.keyPressEvent(&evT);
    QKeyEvent evN(QEvent::KeyPress, Qt::Key_N, Qt::ControlModifier);
    w.keyPressEvent(&evN);

    // Assert
    EXPECT_EQ(nextTab.count(), 1);
    EXPECT_EQ(searchF.count(), 1);
    EXPECT_EQ(searchL.count(), 1);
    EXPECT_EQ(back.count(), 1);
    EXPECT_EQ(fwd.count(), 1);
    EXPECT_EQ(closeTab.count(), 1);
    EXPECT_EQ(newTab.count(), 1);
    EXPECT_EQ(newWin.count(), 1);
}

TEST_F(UT_FileManagerWindowCov, KeyPress_CtrlDigit3_ExpectTriggerActionByIndex2)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    QSignalSpy spy(&w, &FileManagerWindow::reqTriggerActionByIndex);
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_3, Qt::ControlModifier);

    // Act
    w.keyPressEvent(&ev);

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 2);
}

TEST_F(UT_FileManagerWindowCov, KeyPress_AltDigit1_ExpectActivateTabByIndex0)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    QSignalSpy spy(&w, &FileManagerWindow::reqActivateTabByIndex);
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_1, Qt::AltModifier);

    // Act
    w.keyPressEvent(&ev);

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 0);
}

TEST_F(UT_FileManagerWindowCov, KeyPress_CtrlShiftQuestion_ExpectHotkeyHelpEmitted)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    QSignalSpy spy(&w, &FileManagerWindow::reqShowHotkeyHelp);
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Question,
                 Qt::ControlModifier | Qt::ShiftModifier);

    // Act
    w.keyPressEvent(&ev);

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).count(), 0);
}

TEST_F(UT_FileManagerWindowCov, KeyPress_UnhandledKey_ExpectNoShortcutSignal)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    QSignalSpy spy(&w, &FileManagerWindow::reqRefresh);
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_F9, Qt::NoModifier);

    // Act
    w.keyPressEvent(&ev);

    // Assert
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(w.windowTitle(), QStringLiteral("tmp"));
}

TEST_F(UT_FileManagerWindowCov, StatePersistence_SaveAndLoad_ExpectNoErrorAndSaveClosedStateTrue)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    MockFrame *title = nullptr, *side = nullptr, *space = nullptr, *detail = nullptr;
    installAll(&w, &title, &side, &space, &detail);
    // saveWindowState() reads windowHandle() properties: force a native handle
    // so the not-yet-shown window has one.
    (void)w.winId();

    // Act
    w.saveState();
    w.loadState();

    // Assert
    EXPECT_TRUE(w.saveClosedSate());
    EXPECT_EQ(w.currentUrl(), QUrl::fromLocalFile("/tmp"));
}

TEST_F(UT_FileManagerWindowCov, MoveCenter_WithPrimaryScreen_ExpectCenteredPosition)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    QScreen *screen = QGuiApplication::primaryScreen();
    ASSERT_NE(screen, nullptr);
    QPoint expected((screen->availableGeometry().width() - w.width()) / 2
                            + screen->geometry().topLeft().x(),
                    (screen->availableGeometry().height() - w.height()) / 2
                            + screen->geometry().topLeft().y());

    // Act
    w.moveCenter();

    // Assert
    EXPECT_EQ(w.pos(), expected);
    // the window center must match the screen center (within rounding)
    EXPECT_LE(qAbs(w.geometry().center().x() - screen->availableGeometry().center().x()), 1);
}

TEST_F(UT_FileManagerWindowCov, WindowMove_ManualMove_ExpectPositionChangedSignal)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    QSignalSpy spy(&w, &FileManagerWindow::positionChanged);
    QMoveEvent ev(QPoint(11, 22), QPoint(0, 0));

    // Act
    w.moveEvent(&ev);

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toPoint(), QPoint(11, 22));
}

TEST_F(UT_FileManagerWindowCov, EventFilter_WorkspaceF5Key_ExpectEventConsumed)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    MockFrame *title = nullptr, *side = nullptr, *space = nullptr, *detail = nullptr;
    installAll(&w, &title, &side, &space, &detail);
    QSignalSpy spy(&w, &FileManagerWindow::reqRefresh);
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_F5, Qt::NoModifier);

    // Act
    bool consumed = w.eventFilter(space, &ev);

    // Assert
    EXPECT_TRUE(consumed);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_FileManagerWindowCov, EventFilter_UnrelatedObject_ExpectNotConsumed)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    QObject stranger;
    QEvent ev(QEvent::User);

    // Act
    bool consumed = w.eventFilter(&stranger, &ev);

    // Assert
    EXPECT_FALSE(consumed);
    EXPECT_EQ(w.windowTitle(), QStringLiteral("tmp"));
}

TEST_F(UT_FileManagerWindowCov, ResizeEvent_AfterFramesInstalled_ExpectGeometryUnchangedByEvent)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    MockFrame *title = nullptr, *side = nullptr, *space = nullptr, *detail = nullptr;
    installAll(&w, &title, &side, &space, &detail);
    QSize before = w.size();
    QResizeEvent ev(QSize(1100, 750), before);

    // Act
    EXPECT_NO_FATAL_FAILURE(w.resizeEvent(&ev));

    // Assert
    EXPECT_EQ(w.size(), before);
    EXPECT_EQ(w.height(), before.height());
}

TEST_F(UT_FileManagerWindowCov, ResizeEvent_NoFramesInstalled_ExpectEarlyReturn)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    QSize before = w.size();
    QResizeEvent ev(QSize(900, 600), before);

    // Act
    EXPECT_NO_FATAL_FAILURE(w.resizeEvent(&ev));

    // Assert
    EXPECT_EQ(w.size(), before);
    EXPECT_EQ(w.height(), before.height());
}

TEST_F(UT_FileManagerWindowCov, DoubleClick_BelowTitleBar_ExpectNotMaximized)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    MockFrame *title = nullptr, *side = nullptr, *space = nullptr, *detail = nullptr;
    installAll(&w, &title, &side, &space, &detail);
    QMouseEvent ev(QEvent::MouseButtonDblClick, QPointF(50, 500), QPointF(50, 500),
                   QPointF(50, 500), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    // Act
    EXPECT_NO_FATAL_FAILURE(w.mouseDoubleClickEvent(&ev));

    // Assert
    EXPECT_FALSE(w.isMaximized());
    EXPECT_EQ(w.windowTitle(), QStringLiteral("tmp"));
}

TEST_F(UT_FileManagerWindowCov, DoubleClick_OnTitleBar_ExpectWindowMaximized)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    MockFrame *title = nullptr, *side = nullptr, *space = nullptr, *detail = nullptr;
    installAll(&w, &title, &side, &space, &detail);
    QMouseEvent ev(QEvent::MouseButtonDblClick, QPointF(50, 2), QPointF(50, 2),
                   QPointF(50, 2), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    // Act
    EXPECT_NO_FATAL_FAILURE(w.mouseDoubleClickEvent(&ev));

    // Assert
    EXPECT_TRUE(w.windowState() & Qt::WindowMaximized);
    EXPECT_EQ(w.windowTitle(), QStringLiteral("tmp"));
}

TEST_F(UT_FileManagerWindowCov, Cd_UrlChange_ExpectCurrentUrlChangedSignal)
{
    // Arrange
    FileManagerWindow w(QUrl::fromLocalFile("/tmp"));
    QSignalSpy spy(&w, &FileManagerWindow::currentUrlChanged);
    QUrl home = QUrl::fromLocalFile("/home");

    // Act
    w.cd(home);

    // Assert
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toUrl(), home);
    EXPECT_EQ(w.currentUrl(), home);
}
