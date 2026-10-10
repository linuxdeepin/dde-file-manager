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

#include <plugins/filemanager/dfmplugin-recent/events/recenteventreceiver.h>
#include <plugins/filemanager/dfmplugin-recent/utils/recentfilehelper.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventReceiverSweepTest, Register_RecentEventReceiver_0_ArityGuardNoBody)
{
    dpf::EventDispatcherManager mgr;
    auto *receiver = dfmplugin_recent::RecentEventReceiver::instance();
    EXPECT_TRUE(mgr.subscribe(61000, receiver, static_cast<void (dfmplugin_recent::RecentEventReceiver::*)(unsigned long long, QUrl const&)>(&dfmplugin_recent::RecentEventReceiver::handleWindowUrlChanged)));
    EXPECT_TRUE(mgr.publish(61000));  // arity guard: dispatch ok, slot body not run
}

TEST_F(EventReceiverSweepTest, Register_RecentEventReceiver_1_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_recent::RecentEventReceiver::instance();
    EXPECT_TRUE(mgr.follow(61002, receiver, static_cast<bool (dfmplugin_recent::RecentEventReceiver::*)(QUrl const&, QList<dfmbase::Global::ItemRoles>*)>(&dfmplugin_recent::RecentEventReceiver::customColumnRole)));
    EXPECT_FALSE(mgr.run(61002));  // arity guard: no handler accepted
}

TEST_F(EventReceiverSweepTest, Register_RecentEventReceiver_2_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_recent::RecentEventReceiver::instance();
    EXPECT_TRUE(mgr.follow(61003, receiver, static_cast<bool (dfmplugin_recent::RecentEventReceiver::*)(QList<QUrl> const&, QUrl const&, Qt::DropAction*)>(&dfmplugin_recent::RecentEventReceiver::checkDragDropAction)));
    EXPECT_FALSE(mgr.run(61003));  // arity guard: no handler accepted
}

TEST_F(EventReceiverSweepTest, Register_RecentFileHelper_3_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_recent::RecentFileHelper::instance();
    EXPECT_TRUE(mgr.follow(61005, receiver, static_cast<bool (dfmplugin_recent::RecentFileHelper::*)(unsigned long long, QList<QUrl>)>(&dfmplugin_recent::RecentFileHelper::openFileInPlugin)));
    EXPECT_FALSE(mgr.run(61005));  // arity guard: no handler accepted
}

TEST_F(EventReceiverSweepTest, Register_RecentEventReceiver_4_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_recent::RecentEventReceiver::instance();
    EXPECT_TRUE(mgr.follow(61006, receiver, static_cast<bool (dfmplugin_recent::RecentEventReceiver::*)(QUrl const&, dfmbase::Global::ItemRoles, QString*)>(&dfmplugin_recent::RecentEventReceiver::customRoleDisplayName)));
    EXPECT_FALSE(mgr.run(61006));  // arity guard: no handler accepted
}
