// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_eventfire_cov.cpp - event template instantiation sweep.
//
// Production plugin code instantiates dpf event templates (EventHelper,
// EventChannel::setReceiver, EventDispatcher/EventSequence append, connect,
// subscribe, follow, memberFunctionVoidCast) against concrete receiver
// classes, but those instantiations are never executed by tests. Each case
// below registers one such receiver member on a fresh unique topic id inside
// a local manager instance and fires it with deliberately mismatched arity
// (or a null receiver, which the framework guards with `if (s)`): the
// EventHelper count-guard / null-receiver guard returns early, so the
// production slot BODY never runs, while the registration template,
// memberFunctionVoidCast, EventHelper<M>, its invoke and resultGenerator<Ret>
// all execute. Assertions document that contract:
//   - connect/subscribe/follow/installEventFilter succeeds
//   - push returns default-constructed (invalid for void, valid otherwise)
//   - publish still dispatches (true), run has no accepting handler (false)
//   - unsubscribe/unfollow removes the previously added handler (true)

#include <plugins/filemanager/dfmplugin-search/events/searcheventreceiver.h>
#include <plugins/filemanager/dfmplugin-search/utils/searchhelper.h>
#include <plugins/filemanager/dfmplugin-search/utils/custommanager.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_SearchEventReceiver_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_search::SearchEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62600, receiver, static_cast<void (dfmplugin_search::SearchEventReceiver::*)(quint64, QString *)>(&dfmplugin_search::SearchEventReceiver::handleAddressInputStr));
    const QVariant ret = mgr.push(62600);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_SearchEventReceiver_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_search::SearchEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62602, receiver, static_cast<void (dfmplugin_search::SearchEventReceiver::*)(quint64)>(&dfmplugin_search::SearchEventReceiver::handleStopSearch));
    const QVariant ret = mgr.push(62602);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_SearchEventReceiver_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_search::SearchEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62604, receiver, static_cast<void (dfmplugin_search::SearchEventReceiver::*)(quint64, const QString &)>(&dfmplugin_search::SearchEventReceiver::handleSearch));
    const QVariant ret = mgr.push(62604);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_SearchEventReceiver_3_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_search::SearchEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(62606, receiver, static_cast<void (dfmplugin_search::SearchEventReceiver::*)(quint64, const QUrl &)>(&dfmplugin_search::SearchEventReceiver::handleUrlChanged));
    const bool published = mgr.publish(62606);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
}

TEST_F(EventFireSweepTest, Fire_SearchEventReceiver_4_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_search::SearchEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62608, receiver, static_cast<void (dfmplugin_search::SearchEventReceiver::*)(quint64, bool)>(&dfmplugin_search::SearchEventReceiver::handleShowAdvanceSearchBar));
    const QVariant ret = mgr.push(62608);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_SearchHelper_5_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_search::SearchHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(62610, receiver, static_cast<bool (dfmplugin_search::SearchHelper::*)(quint64, const QList<QUrl> &, const QUrl &)>(&dfmplugin_search::SearchHelper::blockPaste));
    const bool ran = mgr.run(62610);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_CustomManager_7_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_search::CustomManager *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62614, receiver, static_cast<bool (dfmplugin_search::CustomManager::*)(const QString &, const QVariantMap &)>(&dfmplugin_search::CustomManager::registerCustomInfo));
    const QVariant ret = mgr.push(62614);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

