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

#include <plugins/filemanager/dfmplugin-vault/events/vaulteventreceiver.h>
#include <plugins/filemanager/dfmplugin-vault/utils/vaultfilehelper.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweep2Test : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};


TEST_F(EventReceiverSweep2Test, Register_VaultFileHelper_1_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_vault::VaultFileHelper::instance();
    EXPECT_TRUE(mgr.follow(61002, receiver, static_cast<bool (dfmplugin_vault::VaultFileHelper::*)(QList<QUrl> const&, QUrl const&)>(&dfmplugin_vault::VaultFileHelper::handleDropFiles)));
    EXPECT_FALSE(mgr.run(61002));  // arity guard: no handler accepted
}


TEST_F(EventReceiverSweep2Test, Register_VaultFileHelper_3_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_vault::VaultFileHelper::instance();
    EXPECT_TRUE(mgr.follow(61004, receiver, static_cast<bool (dfmplugin_vault::VaultFileHelper::*)(unsigned long long, QList<QUrl>, std::pair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag>)>(&dfmplugin_vault::VaultFileHelper::renameFilesAddText)));
    EXPECT_FALSE(mgr.run(61004));  // arity guard: no handler accepted
}

TEST_F(EventReceiverSweep2Test, Register_VaultFileHelper_4_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_vault::VaultFileHelper::instance();
    EXPECT_TRUE(mgr.follow(61005, receiver, static_cast<bool (dfmplugin_vault::VaultFileHelper::*)(unsigned long long, QList<QUrl>)>(&dfmplugin_vault::VaultFileHelper::openFileInPlugin)));
    EXPECT_FALSE(mgr.run(61005));  // arity guard: no handler accepted
}

TEST_F(EventReceiverSweep2Test, Register_VaultEventReceiver_5_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_vault::VaultEventReceiver::instance();
    EXPECT_TRUE(mgr.follow(61006, receiver, static_cast<bool (dfmplugin_vault::VaultEventReceiver::*)(QUrl const&, QString*)>(&dfmplugin_vault::VaultEventReceiver::detailViewIcon)));
    EXPECT_FALSE(mgr.run(61006));  // arity guard: no handler accepted
}
