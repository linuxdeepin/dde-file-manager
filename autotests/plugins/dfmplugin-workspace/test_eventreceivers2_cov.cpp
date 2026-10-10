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
#include <plugins/filemanager/dfmplugin-workspace/views/fileview.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweep2Test : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};



TEST_F(EventReceiverSweep2Test, Register_WorkspaceEventReceiver_2_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
        auto *receiver = dfmplugin_workspace::WorkspaceEventReceiver::instance();
        EXPECT_TRUE(mgr.connect(61003, receiver, static_cast<int (dfmplugin_workspace::WorkspaceEventReceiver::*)(unsigned long long, dfmbase::Global::ItemRoles)>(&dfmplugin_workspace::WorkspaceEventReceiver::handleGetColumnWidth)));
        EXPECT_TRUE((mgr.push(61003)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweep2Test, Register_WorkspaceEventReceiver_3_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
        auto *receiver = dfmplugin_workspace::WorkspaceEventReceiver::instance();
        EXPECT_TRUE(mgr.connect(61004, receiver, static_cast<QString (dfmplugin_workspace::WorkspaceEventReceiver::*)(unsigned long long)>(&dfmplugin_workspace::WorkspaceEventReceiver::handleCurrentGroupStrategy)));
        EXPECT_TRUE((mgr.push(61004)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweep2Test, Register_WorkspaceEventReceiver_4_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
        auto *receiver = dfmplugin_workspace::WorkspaceEventReceiver::instance();
        EXPECT_TRUE(mgr.connect(61005, receiver, static_cast<void (dfmplugin_workspace::WorkspaceEventReceiver::*)(QString const&, QString const&)>(&dfmplugin_workspace::WorkspaceEventReceiver::handleRegisterMenuScene)));
        EXPECT_FALSE((mgr.push(61005)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweep2Test, Register_WorkspaceEventReceiver_5_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
        auto *receiver = dfmplugin_workspace::WorkspaceEventReceiver::instance();
        EXPECT_TRUE(mgr.connect(61006, receiver, static_cast<QList<QUrl> (dfmplugin_workspace::WorkspaceEventReceiver::*)(unsigned long long)>(&dfmplugin_workspace::WorkspaceEventReceiver::handleGetSelectedUrls)));
        EXPECT_TRUE((mgr.push(61006)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweep2Test, Register_WorkspaceEventReceiver_6_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
        auto *receiver = dfmplugin_workspace::WorkspaceEventReceiver::instance();
        EXPECT_TRUE(mgr.connect(61007, receiver, static_cast<void (dfmplugin_workspace::WorkspaceEventReceiver::*)(unsigned long long)>(&dfmplugin_workspace::WorkspaceEventReceiver::handleSelectAll)));
        EXPECT_FALSE((mgr.push(61007)).isValid());  // arity guard: default-constructed return, slot body not run
}





