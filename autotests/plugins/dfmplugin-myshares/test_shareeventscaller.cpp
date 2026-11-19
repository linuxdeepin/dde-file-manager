// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "events/shareeventscaller.h"
#include "dfmplugin_myshares_global.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QList>
#include <QObject>
#include <QPair>
#include <QVariantHash>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

// exact template instantiations used by shareeventscaller.cpp
using PublishQUrl = bool (EventDispatcherManager::*)(int, QUrl);   // kOpenNewWindow
using PublishWinQUrlRef = bool (EventDispatcherManager::*)(int, quint64, QUrl &);   // kChangeCurrentUrl / kOpenNewTab(first())
using PublishWinIntRRef = bool (EventDispatcherManager::*)(int, quint64, int &&);   // kSwitchViewMode
using PublishWinConstQUrlRef = bool (EventDispatcherManager::*)(int, quint64, const QUrl &);   // sendOpenTab
using PushQString = QVariant (EventChannelManager::*)(const QString &, const QString &, QString);   // sendCancelSharing
using PushUrlsHashRRef = QVariant (EventChannelManager::*)(const QString &, const QString &, QList<QUrl>, QVariantHash &&);   // sendShowProperty
using PushWinId = QVariant (EventChannelManager::*)(const QString &, const QString &, quint64);   // sendCheckTabAddable

class ShareEventsCallerTestSink : public QObject
{
public:
    bool tabAddableResult { true };

    QList<QPair<quint64, QUrl>> currentUrlEvents;
    QList<QUrl> newWindowEvents;
    QList<QPair<quint64, QUrl>> newTabEvents;
    QList<QPair<quint64, int>> viewModeEvents;
    QString removedSharePath;
    QList<QUrl> propertyUrls;
    quint64 lastTabAddableWinId { 0 };

    bool changeCurrentUrl(quint64 winId, const QUrl &url)
    {
        currentUrlEvents.append({ winId, url });
        return true;
    }

    bool openNewWindow(const QUrl &url)
    {
        newWindowEvents.append(url);
        return true;
    }

    bool openNewTab(quint64 winId, const QUrl &url)
    {
        newTabEvents.append({ winId, url });
        return true;
    }

    bool switchViewMode(quint64 winId, int mode)
    {
        viewModeEvents.append({ winId, mode });
        return true;
    }

    bool removeShare(const QString &path)
    {
        removedSharePath = path;
        return true;
    }

    bool showProperty(const QList<QUrl> &urls, const QVariantHash &)
    {
        propertyUrls = urls;
        return true;
    }

    bool tabAddable(quint64 winId)
    {
        lastTabAddableWinId = winId;
        return tabAddableResult;
    }
};

class UT_ShareEventsCaller : public testing::Test
{
public:
    virtual void SetUp() override
    {
        if (Event::instance()->eventType("dfmplugin_dirshare", "slot_Share_RemoveShare") == EventTypeScope::kInValid)
            Event::instance()->registerEventType(EventStratege::kSlot, "dfmplugin_dirshare", "slot_Share_RemoveShare");
        if (Event::instance()->eventType("dfmplugin_propertydialog", "slot_PropertyDialog_Show") == EventTypeScope::kInValid)
            Event::instance()->registerEventType(EventStratege::kSlot, "dfmplugin_propertydialog", "slot_PropertyDialog_Show");
        if (Event::instance()->eventType("dfmplugin_titlebar", "slot_Tab_Addable") == EventTypeScope::kInValid)
            Event::instance()->registerEventType(EventStratege::kSlot, "dfmplugin_titlebar", "slot_Tab_Addable");

        dpfSlotChannel->connect("dfmplugin_dirshare", "slot_Share_RemoveShare", &sink, &ShareEventsCallerTestSink::removeShare);
        dpfSlotChannel->connect("dfmplugin_propertydialog", "slot_PropertyDialog_Show", &sink, &ShareEventsCallerTestSink::showProperty);
        dpfSlotChannel->connect("dfmplugin_titlebar", "slot_Tab_Addable", &sink, &ShareEventsCallerTestSink::tabAddable);

        dpfSignalDispatcher->subscribe(int(GlobalEventType::kChangeCurrentUrl), &sink, &ShareEventsCallerTestSink::changeCurrentUrl);
        dpfSignalDispatcher->subscribe(int(GlobalEventType::kOpenNewWindow), &sink, &ShareEventsCallerTestSink::openNewWindow);
        dpfSignalDispatcher->subscribe(int(GlobalEventType::kOpenNewTab), &sink, &ShareEventsCallerTestSink::openNewTab);
        dpfSignalDispatcher->subscribe(int(GlobalEventType::kSwitchViewMode), &sink, &ShareEventsCallerTestSink::switchViewMode);
    }

    virtual void TearDown() override
    {
        dpfSignalDispatcher->unsubscribe(int(GlobalEventType::kChangeCurrentUrl), &sink, &ShareEventsCallerTestSink::changeCurrentUrl);
        dpfSignalDispatcher->unsubscribe(int(GlobalEventType::kOpenNewWindow), &sink, &ShareEventsCallerTestSink::openNewWindow);
        dpfSignalDispatcher->unsubscribe(int(GlobalEventType::kOpenNewTab), &sink, &ShareEventsCallerTestSink::openNewTab);
        dpfSignalDispatcher->unsubscribe(int(GlobalEventType::kSwitchViewMode), &sink, &ShareEventsCallerTestSink::switchViewMode);
        dpfSlotChannel->disconnect("dfmplugin_dirshare", "slot_Share_RemoveShare");
        dpfSlotChannel->disconnect("dfmplugin_propertydialog", "slot_PropertyDialog_Show");
        dpfSlotChannel->disconnect("dfmplugin_titlebar", "slot_Tab_Addable");
        stub.clear();
    }

    stub_ext::StubExt stub;
    ShareEventsCallerTestSink sink;
};

using namespace dfmplugin_myshares;
using namespace dpf;
DFMBASE_USE_NAMESPACE

TEST_F(UT_ShareEventsCaller, SendOpenDirs_EmptyUrls_NoPublish)
{
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenDirs(0, {}, ShareEventsCaller::OpenMode::kOpenInCurrentWindow));
}

TEST_F(UT_ShareEventsCaller, SendOpenDirs_UsershareUrls_ConvertedToFileAndPublished)
{
    QList<QUrl> urls { QUrl("usershare:///test") };
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenDirs(0, urls, ShareEventsCaller::OpenMode::kOpenInCurrentWindow));
    ASSERT_EQ(1u, sink.currentUrlEvents.count());
    EXPECT_EQ(0u, sink.currentUrlEvents.first().first);
    EXPECT_EQ(QUrl("file:///test"), sink.currentUrlEvents.first().second);   // converted to file scheme

    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenDirs(0, urls, ShareEventsCaller::OpenMode::kOpenInNewWindow));
    ASSERT_EQ(1u, sink.newWindowEvents.count());
    EXPECT_EQ(QUrl("file:///test"), sink.newWindowEvents.first());

    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenDirs(7, urls, ShareEventsCaller::OpenMode::kOpenInNewTab));
    ASSERT_EQ(1u, sink.newTabEvents.count());
    EXPECT_EQ(7u, sink.newTabEvents.first().first);
    EXPECT_EQ(QUrl("file:///test"), sink.newTabEvents.first().second);

    // more than one url: always open each in new window
    urls << QUrl("usershare:///hello");
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenDirs(0, urls, ShareEventsCaller::OpenMode::kOpenInNewTab));
    EXPECT_EQ(3u, sink.newWindowEvents.count());
    EXPECT_EQ(QUrl("file:///test"), sink.newWindowEvents.at(1));
    EXPECT_EQ(QUrl("file:///hello"), sink.newWindowEvents.at(2));
}

TEST_F(UT_ShareEventsCaller, SendCancelSharing_PushesRemoveShareWithPath)
{
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendCancelSharing(QUrl()));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendCancelSharing(QUrl("file:///")));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendCancelSharing(QUrl("usershare:///hello")));
    EXPECT_EQ(QString("/hello"), sink.removedSharePath);
}

TEST_F(UT_ShareEventsCaller, SendShowProperty_PushesPropertyDialogShow)
{
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendShowProperty({ QUrl() }));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendShowProperty({ QUrl("file:///") }));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendShowProperty({ QUrl("file:///"), QUrl("usershare:///hello") }));
    EXPECT_EQ(2u, sink.propertyUrls.count());
    EXPECT_EQ(QUrl("usershare:///hello"), sink.propertyUrls.last());
}

TEST_F(UT_ShareEventsCaller, SendSwitchDisplayMode_PublishesViewModeWithWinId)
{
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(0, Global::ViewMode::kNoneMode));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(0, Global::ViewMode::kIconMode));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(1, Global::ViewMode::kListMode));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(1, Global::ViewMode::kExtendMode));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(1, Global::ViewMode::kAllViewMode));
    ASSERT_EQ(5u, sink.viewModeEvents.count());
    EXPECT_EQ(int(Global::ViewMode::kNoneMode), sink.viewModeEvents.first().second);
    EXPECT_EQ(int(Global::ViewMode::kAllViewMode), sink.viewModeEvents.last().second);
    EXPECT_EQ(1u, sink.viewModeEvents.last().first);
}

TEST_F(UT_ShareEventsCaller, SendOpenWindow_PublishesOpenNewWindow)
{
    QUrl url("usershare:///hello");
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenWindow(url));
    ASSERT_EQ(1u, sink.newWindowEvents.count());
    EXPECT_EQ(url, sink.newWindowEvents.first());
}

TEST_F(UT_ShareEventsCaller, SendOpenTab_PublishesOpenNewTabWithWinId)
{
    QUrl url("usershare:///world");
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenTab(42, url));
    ASSERT_EQ(1u, sink.newTabEvents.count());
    EXPECT_EQ(42u, sink.newTabEvents.first().first);
    EXPECT_EQ(url, sink.newTabEvents.first().second);
}

TEST_F(UT_ShareEventsCaller, SendCheckTabAddable_PushesTabAddableReturnsSlotResult)
{
    sink.tabAddableResult = true;
    EXPECT_TRUE(ShareEventsCaller::sendCheckTabAddable(9));
    EXPECT_EQ(9u, sink.lastTabAddableWinId);

    sink.tabAddableResult = false;
    EXPECT_FALSE(ShareEventsCaller::sendCheckTabAddable(10));
    EXPECT_EQ(10u, sink.lastTabAddableWinId);
}
