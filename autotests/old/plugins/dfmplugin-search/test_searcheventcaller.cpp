// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>

#include "events/searcheventcaller.h"
#include "utils/searchhelper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/filemanagerwindow.h>

#include <dfm-framework/event/eventdispatcher.h>
#include <dfm-framework/event/eventchannel.h>

#include "stubext.h"

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DPF_USE_NAMESPACE

class TestSearchEventCaller : public testing::Test
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

// ========== SendChangeCurrentUrl Tests ==========
TEST_F(TestSearchEventCaller, SendChangeCurrentUrl)
{
    quint64 winId = 12345;
    QUrl url = QUrl::fromLocalFile("/home/test");

    bool signalPublished = false;
    typedef bool (EventDispatcherManager::*Publish)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<Publish>(&EventDispatcherManager::publish),
                   [&signalPublished] {
                       __DBG_STUB_INVOKE__
                       signalPublished = true;
                       return true;
                   });

    EXPECT_NO_FATAL_FAILURE(SearchEventCaller::sendChangeCurrentUrl(winId, url));
    EXPECT_TRUE(signalPublished);
}

// ========== SendShowAdvanceSearchBar Tests ==========
TEST_F(TestSearchEventCaller, SendShowAdvanceSearchBar)
{
    quint64 winId = 12345;
    bool visible = true;

    bool slotPushed = false;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64, QString &&, bool &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push),
                   [&] {
                       __DBG_STUB_INVOKE__
                       slotPushed = true;
                       return QVariant(true);
                   });

    EXPECT_NO_FATAL_FAILURE(SearchEventCaller::sendShowAdvanceSearchBar(winId, visible));
    EXPECT_TRUE(slotPushed);
}

// ========== SendShowAdvanceSearchButton Tests ==========
TEST_F(TestSearchEventCaller, SendShowAdvanceSearchButton)
{
    quint64 winId = 12345;
    bool visible = true;

    bool slotPushed = false;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64, bool &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push),
                   [&slotPushed] {
                       __DBG_STUB_INVOKE__
                       slotPushed = true;
                       return QVariant(true);
                   });

    EXPECT_NO_FATAL_FAILURE(SearchEventCaller::sendShowAdvanceSearchButton(winId, visible));
    EXPECT_TRUE(slotPushed);
}

// ========== SendStartSpinner Tests ==========
TEST_F(TestSearchEventCaller, SendStartSpinner)
{
    quint64 winId = 12345;

    bool slotPushed = false;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push),
                   [&slotPushed] {
                       __DBG_STUB_INVOKE__
                       slotPushed = true;
                       return QVariant(true);
                   });

    EXPECT_NO_FATAL_FAILURE(SearchEventCaller::sendStartSpinner(winId));
    EXPECT_TRUE(slotPushed);
}

// ========== SendStopSpinner Tests ==========
TEST_F(TestSearchEventCaller, SendStopSpinner_WithValidWindow_PushesSlot)
{
    quint64 winId = 12345;

    // Mock window exists
    FileManagerWindow w(QUrl::fromLocalFile("/"));
    stub.set_lamda(&FileManagerWindowsManager::findWindowById,
                   [&w] {
                       __DBG_STUB_INVOKE__
                       return &w;
                   });

    bool slotPushed = false;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push),
                   [&slotPushed] {
                       __DBG_STUB_INVOKE__
                       slotPushed = true;
                       return true;
                   });

    EXPECT_NO_FATAL_FAILURE(SearchEventCaller::sendStopSpinner(winId));
    EXPECT_TRUE(slotPushed);
}
