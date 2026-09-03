// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Integration-style test: register hook event types then run the recent plugin's
// REAL followEvents()/bindEvents() (not stubbed) so the production EventHelper<M>
// template instantiations in recent.cpp actually execute -> cover framework
// event templates (eventhelper.h / invokehelper.h) attributed to the headers.
#include "stubext.h"

#include "recent.h"
#include "utils/recentmanager.h"
#include "utils/recentfilehelper.h"
#include "events/recenteventreceiver.h"

#include <dfm-framework/dpf.h>
#include <dfm-framework/event/event.h>
#include <dfm-framework/event/eventhelper.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_recent;
using namespace dpf;

Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(bool *)
Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(QList<QVariantMap> *)
Q_DECLARE_METATYPE(QFlags<QFileDevice::Permission>)

// Register every hook event type that Recent::followEvents() depends on. These
// are normally registered by constructing the *other* plugins' instances (their
// DPF_EVENT_REG_HOOK members), but those libraries are not linked into this
// test binary, so we register the custom event ids manually.
static void registerRecentHookEvents(dpf::Event *evt)
{
    using S = dpf::EventStratege;
    // workspace hooks
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_Model_FetchCustomColumnRoles");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_Model_FetchCustomRoleDisplayName");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_Delegate_CheckTransparent");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_DragDrop_CheckDragDropAction");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_DragDrop_FileDrop");
    // detailspace hook
    evt->registerEventType(S::kHook, "dfmplugin_detailspace", "hook_Icon_Fetch");
    // titlebar hook
    evt->registerEventType(S::kHook, "dfmplugin_titlebar", "hook_Crumb_Seprate");
    // propertydialog hook
    evt->registerEventType(S::kHook, "dfmplugin_propertydialog", "hook_PropertyDialog_Disable");
    // fileoperations hooks
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_CutToFile");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_CopyFile");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_MoveToTrash");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_DeleteFile");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_WriteUrlsToClipboard");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_OpenInTerminal");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_SetPermission");
}

class RecentFollowEventsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        registerRecentHookEvents(dpf::Event::instance());
        // Only stub the genuinely unsafe paths; let followEvents/bindEvents run for real.
        stub.set_lamda(&RecentManager::init, [] { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&Recent::bindWindows, [] { __DBG_STUB_INVOKE__ });
    }
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
    Recent ins;
};

// Run the real followEvents + bindEvents path (initialize calls both) so the
// EventHelper<M> template instantiations in recent.cpp execute and get covered.
TEST_F(RecentFollowEventsTest, Initialize_RunsRealFollowAndBindEvents_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(ins.initialize());
}
