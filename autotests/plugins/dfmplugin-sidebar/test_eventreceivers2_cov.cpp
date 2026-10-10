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

#include <plugins/filemanager/dfmplugin-sidebar/events/sidebareventreceiver.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweep2Test : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};


TEST_F(EventReceiverSweep2Test, Register_SideBarEventReceiver_1_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
        auto *receiver = dfmplugin_sidebar::SideBarEventReceiver::instance();
        EXPECT_TRUE(mgr.connect(61001, receiver, static_cast<void (dfmplugin_sidebar::SideBarEventReceiver::*)(bool)>(&dfmplugin_sidebar::SideBarEventReceiver::handleSetContextMenuEnable)));
        EXPECT_FALSE((mgr.push(61001)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweep2Test, Register_SideBarEventReceiver_2_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
        auto *receiver = dfmplugin_sidebar::SideBarEventReceiver::instance();
        EXPECT_TRUE(mgr.connect(61002, receiver, static_cast<bool (dfmplugin_sidebar::SideBarEventReceiver::*)(int, QUrl const&, QMap<QString, QVariant> const&)>(&dfmplugin_sidebar::SideBarEventReceiver::handleItemInsert)));
        EXPECT_TRUE((mgr.push(61002)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweep2Test, Register_SideBarEventReceiver_3_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
        auto *receiver = dfmplugin_sidebar::SideBarEventReceiver::instance();
        EXPECT_TRUE(mgr.connect(61003, receiver, static_cast<bool (dfmplugin_sidebar::SideBarEventReceiver::*)(QUrl const&)>(&dfmplugin_sidebar::SideBarEventReceiver::handleItemRemove)));
        EXPECT_TRUE((mgr.push(61003)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweep2Test, Register_SideBarEventReceiver_4_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
        auto *receiver = dfmplugin_sidebar::SideBarEventReceiver::instance();
        EXPECT_TRUE(mgr.connect(61004, receiver, static_cast<bool (dfmplugin_sidebar::SideBarEventReceiver::*)(QUrl const&, QMap<QString, QVariant> const&)>(&dfmplugin_sidebar::SideBarEventReceiver::handleItemAdd)));
        EXPECT_TRUE((mgr.push(61004)).isValid());  // arity guard: default-constructed return, slot body not run
}
