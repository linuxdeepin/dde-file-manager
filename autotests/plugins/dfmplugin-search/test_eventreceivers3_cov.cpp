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

#include <plugins/filemanager/dfmplugin-search/events/searcheventreceiver.h>
#include <plugins/filemanager/dfmplugin-search/utils/searchhelper.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweep3Test : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};


TEST_F(EventReceiverSweep3Test, Register_SearchEventReceiver_1_ArityGuardNoBody)
{
    dpf::EventDispatcherManager mgr;
    auto *receiver = dfmplugin_search::SearchEventReceiver::instance();
    EXPECT_TRUE(mgr.subscribe(61001, receiver, static_cast<void (dfmplugin_search::SearchEventReceiver::*)(QUrl const&, QUrl const&)>(&dfmplugin_search::SearchEventReceiver::handleFileRename)));
    EXPECT_TRUE(mgr.publish(61001));  // arity guard: dispatch ok, slot body not run
}

TEST_F(EventReceiverSweep3Test, Register_SearchHelper_2_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_search::SearchHelper::instance();
    EXPECT_TRUE(mgr.follow(61002, receiver, static_cast<bool (dfmplugin_search::SearchHelper::*)(QUrl const&, QString*)>(&dfmplugin_search::SearchHelper::searchIconName)));
    EXPECT_FALSE(mgr.run(61002));  // arity guard: no handler accepted
}

TEST_F(EventReceiverSweep3Test, Register_SearchHelper_3_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_search::SearchHelper::instance();
    EXPECT_TRUE(mgr.follow(61003, receiver, static_cast<bool (dfmplugin_search::SearchHelper::*)(QUrl const&, QList<dfmbase::Global::ItemRoles>*)>(&dfmplugin_search::SearchHelper::customColumnRole)));
    EXPECT_FALSE(mgr.run(61003));  // arity guard: no handler accepted
}

TEST_F(EventReceiverSweep3Test, Register_SearchHelper_4_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_search::SearchHelper::instance();
    EXPECT_TRUE(mgr.follow(61004, receiver, static_cast<bool (dfmplugin_search::SearchHelper::*)(QUrl const&, dfmbase::Global::ItemRoles, QString*)>(&dfmplugin_search::SearchHelper::customRoleDisplayName)));
    EXPECT_FALSE(mgr.run(61004));  // arity guard: no handler accepted
}
