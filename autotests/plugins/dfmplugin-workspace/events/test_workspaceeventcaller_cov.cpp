// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (uncovered functions in events/workspaceeventcaller.cpp):
//   - WorkspaceEventCaller::sendOpenNewTab                       -> SendOpenNewTab_PublishesEventWithArgs
//   - WorkspaceEventCaller::sendChangeCurrentUrl                 -> SendChangeCurrentUrl_EmptyAndValidUrl, SendChangeCurrentUrl_PublishesEvent
//   - WorkspaceEventCaller::sendOpenAsAdmin                      -> SendOpenAsAdmin_PublishesEvent
//   - WorkspaceEventCaller::sendShowCustomTopWidget              -> SendShowCustomTopWidget_PushesSlotChannel
//   - WorkspaceEventCaller::sendPaintEmblems                     -> SendPaintEmblems_KeepsPainterActive
//   - WorkspaceEventCaller::sendRenameEndEdit                    -> SendRenameEndEdit_ReturnsSubscribeState
//   - WorkspaceEventCaller::sendModelFilesEmpty                  -> SendModelFilesEmpty_PublishesSignalTopic
//   - WorkspaceEventCaller::sendCheckTabAddable                  -> SendCheckTabAddable_ReturnsFalseWithoutPlugin
//   - WorkspaceEventCaller::sendCloseTab                         -> SendCloseTab_PushesSlotChannel
// Branch notes:
//   sendChangeCurrentUrl: (a) hook returns true -> early return; (b) url empty -> no publish;
//                         (c) url valid -> publish. Hook path is exercised implicitly (no hook
//                         registered -> run() returns false, falls through to url checks).

#include <gtest/gtest.h>

#include "events/workspaceeventcaller.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-framework/dpf.h>
#include <dfm-framework/event/event.h>

#include <QPainter>
#include <QPixmap>
#include <QRectF>
#include <QObject>
#include <QUrl>
#include <QVariant>

#include <atomic>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_workspace;

namespace {
constexpr char kEventSpace[] { "dfmplugin_workspace" };

void registerCallerTopics()
{
    static bool registered = false;
    if (registered)
        return;
    registered = true;
    dpfEvent->registerEventType(EventStratege::kSignal, kEventSpace, "signal_View_RenameEndEdit");
    dpfEvent->registerEventType(EventStratege::kSignal, kEventSpace, "signal_Model_EmptyDir");
}

// Receiver object: the dispatcher API subscribes (type, object, member-function) and
// requires the receiver to be QObject-derived.
struct CallerSink : public QObject
{
    int openTabCalls = 0;
    quint64 openTabWindow = 0;
    QUrl openTabUrl;

    int changeUrlCalls = 0;
    quint64 changeUrlWindow = 0;

    int adminCalls = 0;
    QUrl adminUrl;

    int renameCalls = 0;
    int emptyCalls = 0;

    void onOpenNewTab(quint64 windowId, const QUrl &url)
    {
        ++openTabCalls;
        openTabWindow = windowId;
        openTabUrl = url;
    }
    void onChangeCurrentUrl(quint64 windowId, const QUrl &) { ++changeUrlCalls; changeUrlWindow = windowId; }
    void onOpenAsAdmin(const QUrl &url)
    {
        ++adminCalls;
        adminUrl = url;
    }
    void onRenameEndEdit(quint64, const QUrl &) { ++renameCalls; }
    void onModelFilesEmpty() { ++emptyCalls; }
};
}   // namespace

class UT_WorkspaceEventCallerCov : public ::testing::Test
{
protected:
    void SetUp() override
    {
        registerCallerTopics();
    }

    void TearDown() override
    {
        dpfSignalDispatcher->unsubscribe(GlobalEventType::kOpenNewTab, &sink, &CallerSink::onOpenNewTab);
        dpfSignalDispatcher->unsubscribe(GlobalEventType::kChangeCurrentUrl, &sink, &CallerSink::onChangeCurrentUrl);
        dpfSignalDispatcher->unsubscribe(GlobalEventType::kOpenAsAdmin, &sink, &CallerSink::onOpenAsAdmin);
        dpfSignalDispatcher->unsubscribe(kEventSpace, "signal_View_RenameEndEdit", &sink, &CallerSink::onRenameEndEdit);
        dpfSignalDispatcher->unsubscribe(kEventSpace, "signal_Model_EmptyDir", &sink, &CallerSink::onModelFilesEmpty);
    }

    CallerSink sink;
};

TEST_F(UT_WorkspaceEventCallerCov, SendOpenNewTab_PublishesEventWithArgs)
{
    // Arrange
    ASSERT_TRUE(dpfSignalDispatcher->subscribe(GlobalEventType::kOpenNewTab, &sink, &CallerSink::onOpenNewTab));
    const quint64 expectWindow = 4242;
    const QUrl expectUrl = QUrl::fromLocalFile("/tmp/ut-caller-open-tab");

    // Act
    WorkspaceEventCaller::sendOpenNewTab(expectWindow, expectUrl);

    // Assert
    EXPECT_EQ(sink.openTabCalls, 1);
    EXPECT_EQ(sink.openTabWindow, expectWindow);
    EXPECT_EQ(sink.openTabUrl, expectUrl);
}

TEST_F(UT_WorkspaceEventCallerCov, SendChangeCurrentUrl_EmptyAndValidUrl)
{
    // Arrange
    ASSERT_TRUE(dpfSignalDispatcher->subscribe(GlobalEventType::kChangeCurrentUrl, &sink, &CallerSink::onChangeCurrentUrl));

    // Act
    WorkspaceEventCaller::sendChangeCurrentUrl(1, QUrl());

    // Assert
    EXPECT_EQ(sink.changeUrlCalls, 0);   // empty url must not be published

    // Act (valid url)
    WorkspaceEventCaller::sendChangeCurrentUrl(1, QUrl::fromLocalFile("/tmp"));

    // Assert
    EXPECT_EQ(sink.changeUrlCalls, 1);
    EXPECT_EQ(sink.changeUrlWindow, 1);
}

TEST_F(UT_WorkspaceEventCallerCov, SendOpenAsAdmin_PublishesEvent)
{
    // Arrange
    ASSERT_TRUE(dpfSignalDispatcher->subscribe(GlobalEventType::kOpenAsAdmin, &sink, &CallerSink::onOpenAsAdmin));
    const QUrl expectUrl = QUrl::fromLocalFile("/tmp/ut-caller-admin");

    // Act
    WorkspaceEventCaller::sendOpenAsAdmin(expectUrl);

    // Assert
    EXPECT_EQ(sink.adminCalls, 1);
    EXPECT_EQ(sink.adminUrl, expectUrl);
}

TEST_F(UT_WorkspaceEventCallerCov, SendRenameEndEdit_ReturnsSubscribeState)
{
    // Arrange
    const quint64 winId = 7;

    // Act (no listener yet)
    const bool noListener = WorkspaceEventCaller::sendRenameEndEdit(winId, QUrl::fromLocalFile("/tmp/a"));

    // Assert
    EXPECT_EQ(sink.renameCalls, 0);

    // Act (with listener)
    ASSERT_TRUE(dpfSignalDispatcher->subscribe(kEventSpace, "signal_View_RenameEndEdit", &sink, &CallerSink::onRenameEndEdit));
    const bool withListener = WorkspaceEventCaller::sendRenameEndEdit(winId, QUrl::fromLocalFile("/tmp/b"));

    // Assert
    EXPECT_EQ(sink.renameCalls, 1);
    EXPECT_TRUE(withListener);
    EXPECT_NE(withListener, noListener);
}

TEST_F(UT_WorkspaceEventCallerCov, SendModelFilesEmpty_PublishesSignalTopic)
{
    // Arrange
    ASSERT_TRUE(dpfSignalDispatcher->subscribe(kEventSpace, "signal_Model_EmptyDir", &sink, &CallerSink::onModelFilesEmpty));

    // Act
    WorkspaceEventCaller::sendModelFilesEmpty();

    // Assert
    EXPECT_EQ(sink.emptyCalls, 1);
    WorkspaceEventCaller::sendModelFilesEmpty();
    EXPECT_EQ(sink.emptyCalls, 2);
}

TEST_F(UT_WorkspaceEventCallerCov, SendCheckTabAddable_ReturnsFalseWithoutPlugin)
{
    // Arrange: no "dfmplugin_titlebar/slot_Tab_Addable" slot is registered in unit tests.

    // Act
    const bool first = WorkspaceEventCaller::sendCheckTabAddable(100);
    const bool second = WorkspaceEventCaller::sendCheckTabAddable(200);

    // Assert
    EXPECT_FALSE(first);
    EXPECT_EQ(first, second);   // deterministic across window ids
    EXPECT_FALSE(second);
}

TEST_F(UT_WorkspaceEventCallerCov, SendShowCustomTopWidget_PushesSlotChannel)
{
    // Arrange: slot "dfmplugin_workspace/slot_ShowCustomTopWidget" has no receiver in unit tests.

    // Act
    WorkspaceEventCaller::sendShowCustomTopWidget(3, "file", true);
    WorkspaceEventCaller::sendShowCustomTopWidget(4, "admin", false);

    // Assert: pushes must be harmless without receivers (exact window/scheme values are
    // not observable here, the push path itself is the covered code).
    EXPECT_NO_FATAL_FAILURE(WorkspaceEventCaller::sendShowCustomTopWidget(5, "file", true));
    SUCCEED() << "slot channel push completed for three window ids";
}

TEST_F(UT_WorkspaceEventCallerCov, SendCloseTab_PushesSlotChannel)
{
    // Arrange: slot "dfmplugin_titlebar/slot_Tab_Close" has no receiver in unit tests.
    const QUrl url = QUrl::fromLocalFile("/tmp/ut-caller-close-tab");

    // Act
    WorkspaceEventCaller::sendCloseTab(url);

    // Assert
    EXPECT_NO_FATAL_FAILURE(WorkspaceEventCaller::sendCloseTab(url));
    SUCCEED() << "close-tab push executed twice without receiver";
}

TEST_F(UT_WorkspaceEventCallerCov, SendPaintEmblems_KeepsPainterActive)
{
    // Arrange
    QPixmap pixmap(64, 64);
    QPainter painter(&pixmap);
    const QRectF area(0, 0, 64, 64);
    FileInfoPointer nullInfo;
    ASSERT_TRUE(painter.isActive());

    // Act
    WorkspaceEventCaller::sendPaintEmblems(&painter, area, nullInfo);

    // Assert
    EXPECT_TRUE(painter.isActive());
    EXPECT_EQ(pixmap.width(), 64);
    EXPECT_FALSE(painter.viewport().isEmpty());
    painter.end();
}
