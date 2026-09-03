// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Real-events test: call SmbBrowser::followEvents() directly (initialize() does
// NOT call it; it's invoked on window-open). This runs the 16 follow subscribes
// so production EventHelper<M> follow templates execute. We avoid initialize()
// because bindSetting() -> SettingBackend::instance() asserts Application exists.
#include <gtest/gtest.h>
#include "stubext.h"
#include "dfm_hookreg.h"
#include "smbbrowser.h"
#include <dfm-framework/dpf.h>

DPF_USE_NAMESPACE
using namespace dfmplugin_smbbrowser;

TEST(SmbBrowserRealEventsTest, FollowEvents_RunsRealFollows_NoCrash)
{
    dfmtest_hooks::registerAllHookEvents();
    SmbBrowser ins;
    // followEvents() is private but -fno-access-control disables access checks.
    EXPECT_NO_FATAL_FAILURE(ins.followEvents());
}
