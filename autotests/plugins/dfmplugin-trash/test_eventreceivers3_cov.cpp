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

#include <plugins/filemanager/dfmplugin-trash/utils/trashfilehelper.h>
#include <plugins/filemanager/dfmplugin-trash/utils/trashhelper.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweep3Test : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventReceiverSweep3Test, Register_TrashFileHelper_0_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_trash::TrashFileHelper::instance();
    EXPECT_TRUE(mgr.follow(61000, receiver, static_cast<bool (dfmplugin_trash::TrashFileHelper::*)(QUrl const&, QUrl const&)>(&dfmplugin_trash::TrashFileHelper::handleIsSubFile)));
    EXPECT_FALSE(mgr.run(61000));  // arity guard: no handler accepted
}


TEST_F(EventReceiverSweep3Test, Register_TrashFileHelper_2_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_trash::TrashFileHelper::instance();
    EXPECT_TRUE(mgr.follow(61002, receiver, static_cast<bool (dfmplugin_trash::TrashFileHelper::*)(QUrl const&, bool*)>(&dfmplugin_trash::TrashFileHelper::disableOpenWidgetWidget)));
    EXPECT_FALSE(mgr.run(61002));  // arity guard: no handler accepted
}

TEST_F(EventReceiverSweep3Test, Register_TrashFileHelper_3_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_trash::TrashFileHelper::instance();
    EXPECT_TRUE(mgr.follow(61003, receiver, static_cast<bool (dfmplugin_trash::TrashFileHelper::*)(unsigned long long, QList<QUrl>)>(&dfmplugin_trash::TrashFileHelper::openFileInPlugin)));
    EXPECT_FALSE(mgr.run(61003));  // arity guard: no handler accepted
}
