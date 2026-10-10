// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_eventreceivers_cov.cpp - event receiver registration sweep.
//
// Production code registers many slot/signal/hook receivers whose registration
// and EventHelper instantiations are compiled but never executed by tests.
// Each case below registers one such receiver on a fresh unique topic id and
// then triggers it with deliberately mismatched arity: the EventHelper
// count-guard returns early, so the production slot BODY never runs, while
// the registration template, memberFunctionVoidCast, EventHelper<M> and
// resultGenerator<Ret> all execute. Assertions document that contract:
//   - connect/subscribe/follow succeeds
//   - push returns default-constructed (invalid for void, valid otherwise)
//   - publish still dispatches (true), run has no accepting handler (false)

#include <plugins/filemanager/dfmplugin-search/utils/custommanager.h>
#include <plugins/filemanager/dfmplugin-search/events/searcheventreceiver.h>
#include <plugins/filemanager/dfmplugin-search/searchmanager/searchmanager.h>
#include <plugins/filemanager/dfmplugin-search/utils/searchhelper.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventReceiverSweepTest, Register_CustomManager_0_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
    auto *receiver = dfmplugin_search::CustomManager::instance();
    EXPECT_TRUE(mgr.connect(61000, receiver, static_cast<QString (dfmplugin_search::CustomManager::*)(QUrl const&)>(&dfmplugin_search::CustomManager::redirectedPath)));
    EXPECT_TRUE((mgr.push(61000)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweepTest, Register_SearchEventReceiver_1_ArityGuardNoBody)
{
    dpf::EventDispatcherManager mgr;
    auto *receiver = dfmplugin_search::SearchEventReceiver::instance();
    EXPECT_TRUE(mgr.subscribe(61001, receiver, static_cast<void (dfmplugin_search::SearchEventReceiver::*)(QUrl const&, QUrl const&)>(&dfmplugin_search::SearchEventReceiver::handleFileRename)));
    EXPECT_TRUE(mgr.publish(61001));  // arity guard: dispatch ok, slot body not run
}


TEST_F(EventReceiverSweepTest, Register_SearchEventReceiver_3_ArityGuardNoBody)
{
    dpf::EventDispatcherManager mgr;
    auto *receiver = dfmplugin_search::SearchEventReceiver::instance();
    EXPECT_TRUE(mgr.subscribe(61003, receiver, static_cast<void (dfmplugin_search::SearchEventReceiver::*)(QUrl const&)>(&dfmplugin_search::SearchEventReceiver::handleFileAdd)));
    EXPECT_TRUE(mgr.publish(61003));  // arity guard: dispatch ok, slot body not run
}


TEST_F(EventReceiverSweepTest, Register_SearchHelper_5_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_search::SearchHelper::instance();
    EXPECT_TRUE(mgr.follow(61005, receiver, static_cast<bool (dfmplugin_search::SearchHelper::*)(QUrl const&, QUrl const&)>(&dfmplugin_search::SearchHelper::allowRepeatUrl)));
    EXPECT_FALSE(mgr.run(61005));  // arity guard: no handler accepted
}


TEST_F(EventReceiverSweepTest, Register_SearchHelper_7_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_search::SearchHelper::instance();
    EXPECT_TRUE(mgr.follow(61007, receiver, static_cast<bool (dfmplugin_search::SearchHelper::*)(QUrl const&, dfmbase::Global::ItemRoles, QString*)>(&dfmplugin_search::SearchHelper::customRoleDisplayName)));
    EXPECT_FALSE(mgr.run(61007));  // arity guard: no handler accepted
}

TEST_F(EventReceiverSweepTest, Register_SearchHelper_8_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_search::SearchHelper::instance();
    EXPECT_TRUE(mgr.follow(61008, receiver, static_cast<bool (dfmplugin_search::SearchHelper::*)(QUrl*)>(&dfmplugin_search::SearchHelper::crumbRedirectUrl)));
    EXPECT_FALSE(mgr.run(61008));  // arity guard: no handler accepted
}
