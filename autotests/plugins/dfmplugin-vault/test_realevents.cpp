// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Real-events test: run Vault::initialize() with REAL connectEvent() (the 25
// subscribe/follow calls in VaultEventReceiver::connectEvent) NOT stubbed, so
// production EventHelper<M> templates execute. bindWindows remains stubbed.
#include <gtest/gtest.h>
#include "stubext.h"
#include "dfm_hookreg.h"
#include "vault.h"
#include <dfm-framework/dpf.h>

DPF_USE_NAMESPACE
using namespace dfmplugin_vault;

class VaultRealEventsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dfmtest_hooks::registerAllHookEvents();
        stub.set_lamda(&Vault::bindWindows, [] { __DBG_STUB_INVOKE__ });
        ins = new Vault();
    }
    void TearDown() override { stub.clear(); delete ins; }
    stub_ext::StubExt stub;
    Vault *ins { nullptr };
};

TEST_F(VaultRealEventsTest, Initialize_RunsRealConnectEvent_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(ins->initialize());
}
