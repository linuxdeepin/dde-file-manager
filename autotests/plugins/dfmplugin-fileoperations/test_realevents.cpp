// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Real-events test: run FileOperations::initialize() with REAL initEventHandle()
// and followEvents() (NOT stubbed) so the production EventHelper<M> subscribe/follow
// template instantiations in fileoperations.cpp actually execute -> cover framework
// event template functions (eventdispatcher.h / eventsequence.h / eventhelper.h).
//
// initEventHandle() subscribes use GlobalEventType (always valid, no registration).
// followEvents() follows hooks that need DPF_EVENT_REG_HOOK registration from other
// plugins; we register them via dfm_hookreg.h.
#include <gtest/gtest.h>

#include "stubext.h"
#include "dfm_hookreg.h"

#include "fileoperations.h"

#include <dfm-framework/dpf.h>

DPF_USE_NAMESPACE
using namespace dfmplugin_fileoperations;

class FileOpsRealEventsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dfmtest_hooks::registerAllHookEvents();
        fileOps = new FileOperations();
    }
    void TearDown() override
    {
        delete fileOps;
        fileOps = nullptr;
    }
    FileOperations *fileOps { nullptr };
};

// Run the real initialize() (initEventHandle + followEvents un-stubbed) so the
// EventHelper<M> template instantiations in fileoperations.cpp execute and get covered.
TEST_F(FileOpsRealEventsTest, Initialize_RunsRealInitAndFollowEvents_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(fileOps->initialize());
}
