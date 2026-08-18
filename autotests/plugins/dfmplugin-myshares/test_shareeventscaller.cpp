// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
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

class UT_ShareEventsCaller : public testing::Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
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
    QList<QPair<int, QUrl>> opened;
    stub.set_lamda(static_cast<PublishQUrl>(&EventDispatcherManager::publish),
                   [&opened](EventDispatcherManager *, int type, QUrl url) -> bool {
                       __DBG_STUB_INVOKE__
                       opened.append({ type, url });
                       return true;
                   });
    QList<QPair<int, QPair<quint64, QUrl>>> winEvents;
    stub.set_lamda(static_cast<PublishWinQUrlRef>(&EventDispatcherManager::publish),
                   [&winEvents](EventDispatcherManager *, int type, quint64 winId, QUrl &url) -> bool {
                       __DBG_STUB_INVOKE__
                       winEvents.append({ type, { winId, url } });
                       return true;
                   });

    QList<QUrl> urls { QUrl("usershare:///test") };
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenDirs(0, urls, ShareEventsCaller::OpenMode::kOpenInCurrentWindow));
    ASSERT_EQ(1u, winEvents.count());
    EXPECT_EQ(int(GlobalEventType::kChangeCurrentUrl), winEvents.first().first);
    EXPECT_EQ(0u, winEvents.first().second.first);
    EXPECT_EQ(QUrl("file:///test"), winEvents.first().second.second);   // converted to file scheme

    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenDirs(0, urls, ShareEventsCaller::OpenMode::kOpenInNewWindow));
    ASSERT_EQ(1u, opened.count());
    EXPECT_EQ(int(GlobalEventType::kOpenNewWindow), opened.first().first);
    EXPECT_EQ(QUrl("file:///test"), opened.first().second);

    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenDirs(7, urls, ShareEventsCaller::OpenMode::kOpenInNewTab));
    ASSERT_EQ(2u, winEvents.count());
    EXPECT_EQ(int(GlobalEventType::kOpenNewTab), winEvents.at(1).first);
    EXPECT_EQ(7u, winEvents.at(1).second.first);
    EXPECT_EQ(QUrl("file:///test"), winEvents.at(1).second.second);

    // more than one url: always open each in new window
    urls << QUrl("usershare:///hello");
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenDirs(0, urls, ShareEventsCaller::OpenMode::kOpenInNewTab));
    EXPECT_EQ(3u, opened.count());
    EXPECT_EQ(QUrl("file:///test"), opened.at(1).second);
    EXPECT_EQ(QUrl("file:///hello"), opened.at(2).second);
}

TEST_F(UT_ShareEventsCaller, SendCancelSharing_PushesRemoveShareWithPath)
{
    QString pushedSpace, pushedTopic;
    QString pushedPath;
    stub.set_lamda(static_cast<PushQString>(&EventChannelManager::push),
                   [&](EventChannelManager *, const QString &space, const QString &topic, QString path) -> QVariant {
                       __DBG_STUB_INVOKE__
                       pushedSpace = space;
                       pushedTopic = topic;
                       pushedPath = path;
                       return QVariant();
                   });
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendCancelSharing(QUrl()));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendCancelSharing(QUrl("file:///")));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendCancelSharing(QUrl("usershare:///hello")));
    EXPECT_EQ(QString("dfmplugin_dirshare"), pushedSpace);
    EXPECT_EQ(QString("slot_Share_RemoveShare"), pushedTopic);
    EXPECT_EQ(QString("/hello"), pushedPath);
}

TEST_F(UT_ShareEventsCaller, SendShowProperty_PushesPropertyDialogShow)
{
    QString pushedSpace, pushedTopic;
    QList<QUrl> pushedUrls;
    stub.set_lamda(static_cast<PushUrlsHashRRef>(&EventChannelManager::push),
                   [&](EventChannelManager *, const QString &space, const QString &topic, QList<QUrl> urls, QVariantHash) -> QVariant {
                       __DBG_STUB_INVOKE__
                       pushedSpace = space;
                       pushedTopic = topic;
                       pushedUrls = urls;
                       return QVariant();
                   });

    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendShowProperty({ QUrl() }));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendShowProperty({ QUrl("file:///") }));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendShowProperty({ QUrl("file:///"), QUrl("usershare:///hello") }));
    EXPECT_EQ(QString("dfmplugin_propertydialog"), pushedSpace);
    EXPECT_EQ(QString("slot_PropertyDialog_Show"), pushedTopic);
    EXPECT_EQ(2u, pushedUrls.count());
}

TEST_F(UT_ShareEventsCaller, SendSwitchDisplayMode_PublishesViewModeWithWinId)
{
    QList<QPair<quint64, int>> switched;
    stub.set_lamda(static_cast<PublishWinIntRRef>(&EventDispatcherManager::publish),
                   [&switched](EventDispatcherManager *, int, quint64 winId, int mode) -> bool {
                       __DBG_STUB_INVOKE__
                       switched.append({ winId, mode });
                       return true;
                   });
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(0, Global::ViewMode::kNoneMode));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(0, Global::ViewMode::kIconMode));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(1, Global::ViewMode::kListMode));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(1, Global::ViewMode::kExtendMode));
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendSwitchDisplayMode(1, Global::ViewMode::kAllViewMode));
    ASSERT_EQ(5u, switched.count());
    EXPECT_EQ(int(Global::ViewMode::kNoneMode), switched.first().second);
    EXPECT_EQ(int(Global::ViewMode::kAllViewMode), switched.last().second);
    EXPECT_EQ(1u, switched.last().first);
}

TEST_F(UT_ShareEventsCaller, SendOpenWindow_PublishesOpenNewWindow)
{
    QList<QPair<int, QUrl>> published;
    stub.set_lamda(static_cast<PublishQUrl>(&EventDispatcherManager::publish),
                   [&published](EventDispatcherManager *, int type, QUrl url) -> bool {
                       __DBG_STUB_INVOKE__
                       published.append({ type, url });
                       return true;
                   });

    QUrl url("usershare:///hello");
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenWindow(url));
    ASSERT_EQ(1u, published.count());
    EXPECT_EQ(int(GlobalEventType::kOpenNewWindow), published.first().first);
    EXPECT_EQ(url, published.first().second);
}

TEST_F(UT_ShareEventsCaller, SendOpenTab_PublishesOpenNewTabWithWinId)
{
    QList<QPair<int, QPair<quint64, QUrl>>> published;
    stub.set_lamda(static_cast<PublishWinConstQUrlRef>(&EventDispatcherManager::publish),
                   [&published](EventDispatcherManager *, int type, quint64 winId, const QUrl &url) -> bool {
                       __DBG_STUB_INVOKE__
                       published.append({ type, { winId, url } });
                       return true;
                   });

    QUrl url("usershare:///world");
    EXPECT_NO_FATAL_FAILURE(ShareEventsCaller::sendOpenTab(42, url));
    ASSERT_EQ(1u, published.count());
    EXPECT_EQ(int(GlobalEventType::kOpenNewTab), published.first().first);
    EXPECT_EQ(42u, published.first().second.first);
    EXPECT_EQ(url, published.first().second.second);
}

TEST_F(UT_ShareEventsCaller, SendCheckTabAddable_PushesTabAddableReturnsSlotResult)
{
    bool slotRet = true;
    quint64 pushedWinId = 0;
    stub.set_lamda(static_cast<PushWinId>(&EventChannelManager::push),
                   [&slotRet, &pushedWinId](EventChannelManager *, const QString &space, const QString &topic, quint64 winId) -> QVariant {
                       __DBG_STUB_INVOKE__
                       EXPECT_EQ(QString("dfmplugin_titlebar"), space);
                       EXPECT_EQ(QString("slot_Tab_Addable"), topic);
                       pushedWinId = winId;
                       return QVariant::fromValue(slotRet);
                   });

    EXPECT_TRUE(ShareEventsCaller::sendCheckTabAddable(9));
    EXPECT_EQ(9u, pushedWinId);

    slotRet = false;
    EXPECT_FALSE(ShareEventsCaller::sendCheckTabAddable(10));
    EXPECT_EQ(10u, pushedWinId);
}
