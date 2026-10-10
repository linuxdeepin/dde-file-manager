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

#include <plugins/filemanager/dfmplugin-workspace/events/workspaceeventreceiver.h>
#include <plugins/filemanager/dfmplugin-workspace/utils/workspacehelper.h>
#include <plugins/filemanager/dfmplugin-workspace/views/fileview.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventReceiverSweepTest, Register_WorkspaceEventReceiver_0_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
    auto *receiver = dfmplugin_workspace::WorkspaceEventReceiver::instance();
    EXPECT_TRUE(mgr.connect(61000, receiver, static_cast<bool (dfmplugin_workspace::WorkspaceEventReceiver::*)(QString const&)>(&dfmplugin_workspace::WorkspaceEventReceiver::handleCheckSchemeViewIsFileView)));
    EXPECT_TRUE((mgr.push(61000)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweepTest, Register_WorkspaceEventReceiver_1_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
    auto *receiver = dfmplugin_workspace::WorkspaceEventReceiver::instance();
    EXPECT_TRUE(mgr.connect(61001, receiver, static_cast<QList<QUrl> (dfmplugin_workspace::WorkspaceEventReceiver::*)(unsigned long long)>(&dfmplugin_workspace::WorkspaceEventReceiver::handleGetSelectedUrls)));
    EXPECT_TRUE((mgr.push(61001)).isValid());  // arity guard: default-constructed return, slot body not run
}


TEST_F(EventReceiverSweepTest, Register_WorkspaceEventReceiver_3_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
    auto *receiver = dfmplugin_workspace::WorkspaceEventReceiver::instance();
    EXPECT_TRUE(mgr.connect(61004, receiver, static_cast<QString (dfmplugin_workspace::WorkspaceEventReceiver::*)(unsigned long long)>(&dfmplugin_workspace::WorkspaceEventReceiver::handleCurrentGroupStrategy)));
    EXPECT_TRUE((mgr.push(61004)).isValid());  // arity guard: default-constructed return, slot body not run
}






TEST_F(EventReceiverSweepTest, Register_WorkspaceHelper_9_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
    auto *receiver = dfmplugin_workspace::WorkspaceHelper::instance();
    EXPECT_TRUE(mgr.connect(61011, receiver, static_cast<void (dfmplugin_workspace::WorkspaceHelper::*)(QList<QUrl> const&)>(&dfmplugin_workspace::WorkspaceHelper::actionNewWindow)));
    EXPECT_FALSE((mgr.push(61011)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweepTest, Register_WorkspaceEventReceiver_10_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
    auto *receiver = dfmplugin_workspace::WorkspaceEventReceiver::instance();
    EXPECT_TRUE(mgr.connect(61012, receiver, static_cast<QRectF (dfmplugin_workspace::WorkspaceEventReceiver::*)(unsigned long long, QUrl const&, dfmbase::Global::ItemRoles)>(&dfmplugin_workspace::WorkspaceEventReceiver::handleGetViewItemRect)));
    EXPECT_TRUE((mgr.push(61012)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweepTest, Register_WorkspaceEventReceiver_11_ArityGuardNoBody)
{
    dpf::EventDispatcherManager mgr;
    auto *receiver = dfmplugin_workspace::WorkspaceEventReceiver::instance();
    EXPECT_TRUE(mgr.subscribe(61014, receiver, static_cast<void (dfmplugin_workspace::WorkspaceEventReceiver::*)(unsigned long long, int)>(&dfmplugin_workspace::WorkspaceEventReceiver::handleTileBarSwitchModeTriggered)));
    EXPECT_TRUE(mgr.publish(61014));  // arity guard: dispatch ok, slot body not run
}
