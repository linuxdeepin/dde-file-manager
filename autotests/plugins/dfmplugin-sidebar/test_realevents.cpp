// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Real-events test: run SideBar::initialize() with REAL
// SideBarEventReceiver::bindEvents() (NOT stubbed) so production connect
// templates (EventChannelManager::connect<M>) execute.
#include <gtest/gtest.h>
#include "stubext.h"
#include "dfm_hookreg.h"
#include "sidebar.h"
#include "events/sidebareventreceiver.h"
#include <dfm-framework/dpf.h>

DPF_USE_NAMESPACE
using namespace dfmplugin_sidebar;

class SideBarRealEventsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dfmtest_hooks::registerAllHookEvents();
        stub.set_lamda(ADDR(QThread, start), [](QThread *, QThread::Priority) { __DBG_STUB_INVOKE__ });
        plugin = new SideBar();
    }
    void TearDown() override { stub.clear(); delete plugin; }
    stub_ext::StubExt stub;
    SideBar *plugin { nullptr };
};

TEST_F(SideBarRealEventsTest, Initialize_RunsRealBindEvents_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(plugin->initialize());
}
