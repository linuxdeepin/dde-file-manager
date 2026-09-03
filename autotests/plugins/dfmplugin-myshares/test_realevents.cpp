// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Real-events test: call MyShares::doInitialize() directly (skipping the lazy
// dirshare-plugin-started gate) with REAL followEvents() so production follow
// templates execute. bindWindows remains stubbed.
#include <gtest/gtest.h>
#include "stubext.h"
#include "dfm_hookreg.h"
#include "myshares.h"
#include <dfm-framework/dpf.h>

DPF_USE_NAMESPACE
using namespace dfmplugin_myshares;

class MySharesRealEventsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dfmtest_hooks::registerAllHookEvents();
        stub.set_lamda(&MyShares::bindWindows, [] { __DBG_STUB_INVOKE__ });
        ins = new MyShares();
    }
    void TearDown() override { stub.clear(); delete ins; }
    stub_ext::StubExt stub;
    MyShares *ins { nullptr };
};

TEST_F(MySharesRealEventsTest, DoInitialize_RunsRealFollowEvents_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(ins->doInitialize());
}
