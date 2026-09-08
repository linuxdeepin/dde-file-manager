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
#include "events/recenteventcaller.h"

#include <dfm-framework/dpf.h>
#include <dfm-framework/event/event.h>
#include <dfm-framework/event/eventhelper.h>

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <DDialog>

#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QTemporaryFile>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_recent;
using namespace dpf;
DWIDGET_USE_NAMESPACE

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

// Initialize the plugin exactly once for the whole binary: repeating
// followEvents()/bindEvents() would append duplicate followers and make
// per-signal invocation counts non-deterministic.
static Recent &recentPluginInstance()
{
    static Recent instance;
    static std::once_flag once;
    std::call_once(once, [&instance] { instance.initialize(); });
    return instance;
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
        // removeRecent() shows a modal DDialog and then calls DBus; intercept it
        // (a plain namespace function, safe to stub) so drop/trash hooks stay headless.
        removeRecentCalls = 0;
        stub.set_lamda(&RecentHelper::removeRecent, [this](const QList<QUrl> &) {
            __DBG_STUB_INVOKE__
            ++removeRecentCalls;
        });
        reloadCount = 0;
        stub.set_lamda(&RecentManager::reloadRecent, [this] { __DBG_STUB_INVOKE__ ++reloadCount; });
        // Perform the one-time real followEvents()/bindEvents() registration.
        recentPluginInstance();
    }
    void TearDown() override
    {
        RecentManager::instance()->recentItems.clear();
        stub.clear();
    }
    stub_ext::StubExt stub;
    int reloadCount { 0 };
    int removeRecentCalls { 0 };
};

// The real followEvents + bindEvents path must have run (via the shared
// instance) without crashing, registering every hook/signal follower.
TEST_F(RecentFollowEventsTest, Initialize_RunsRealFollowAndBindEvents_NoCrash)
{
    EXPECT_NE(&recentPluginInstance(), nullptr);
    // Proof that the followers were really registered:
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_propertydialog",
                                     "hook_PropertyDialog_Disable",
                                     RecentHelper::rootUrl()));
}

// Trigger every workspace/detailspace/titlebar/propertydialog hook through the
// real EventSequenceManager so each followed EventHelper<RecentEventReceiver>
// is constructed and invoked with exact-signature arguments.
TEST_F(RecentFollowEventsTest, HookRun_RecentSchemeHooks_InvokeReceiverHandlers)
{
    const QUrl recentRoot { RecentHelper::rootUrl() };
    const QUrl recentItem { "recent:///tmp/follow_item.txt" };

    QList<Global::ItemRoles> roles;
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_workspace", "hook_Model_FetchCustomColumnRoles",
                                     recentRoot, &roles));
    EXPECT_TRUE(roles.contains(Global::ItemRoles::kItemFilePathRole));
    EXPECT_TRUE(roles.contains(Global::ItemRoles::kItemFileLastReadRole));

    QString displayName;
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_workspace", "hook_Model_FetchCustomRoleDisplayName",
                                     recentItem, Global::ItemRoles::kItemFilePathRole, &displayName));
    EXPECT_EQ(displayName.toStdString(), "Path");

    Global::TransparentStatus status { Global::TransparentStatus::kTransparent };
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_workspace", "hook_Delegate_CheckTransparent",
                                     recentItem, &status));
    EXPECT_EQ(status, Global::TransparentStatus::kUntransparent);

    Qt::DropAction action { Qt::IgnoreAction };
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_CheckDragDropAction",
                                     QList<QUrl> { recentItem }, recentRoot, &action));
    EXPECT_EQ(action, Qt::CopyAction);

    // Drops from recent:// into trash:// trigger the removal flow (stubbed).
    const int removesBefore = removeRecentCalls;
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileDrop",
                                     QList<QUrl> { recentItem }, QUrl("trash:///")));
    EXPECT_EQ(removeRecentCalls, removesBefore + 1);

    QString iconName;
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_detailspace", "hook_Icon_Fetch",
                                     recentRoot, &iconName));
    EXPECT_EQ(iconName.toStdString(), "dfm_recent");

    QList<QVariantMap> crumbs;
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_titlebar", "hook_Crumb_Seprate",
                                     recentItem, &crumbs));
    ASSERT_EQ(crumbs.size(), 1);
    EXPECT_EQ(crumbs.first().value("CrumbData_Key_Url").toUrl(), recentRoot);

    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_propertydialog", "hook_PropertyDialog_Disable",
                                     recentRoot));
}

// Trigger every fileoperations hook so the followed
// EventHelper<RecentFileHelper> instantiations are constructed and invoked.
TEST_F(RecentFollowEventsTest, HookRun_FileOperationHooks_InvokeHelperHandlers)
{
    const QUrl recentRoot { RecentHelper::rootUrl() };
    const QUrl recentItem { "recent:///tmp/follow_op.txt" };
    const AbstractJobHandler::JobFlags noFlags {};

    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_CutToFile",
                                     quint64(1), QList<QUrl> { recentItem }, recentRoot, noFlags));
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_CopyFile",
                                     quint64(1), QList<QUrl> { recentItem }, recentRoot, noFlags));
    // moveToTrash triggers the removal flow (stubbed) and succeeds.
    const int removesBefore = removeRecentCalls;
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_MoveToTrash",
                                     quint64(1), QList<QUrl> { recentItem }, noFlags));
    EXPECT_EQ(removeRecentCalls, removesBefore + 1);
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin",
                                     quint64(1), QList<QUrl> { recentItem }));
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_WriteUrlsToClipboard",
                                     quint64(1), ClipBoard::ClipboardAction::kCopyAction,
                                     QList<QUrl> { recentItem }));
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_OpenInTerminal",
                                     quint64(1), QList<QUrl> { recentItem }));

    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString filePath = dir.filePath("perm.txt");
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.close();

    const QUrl recentPerm { RecentHelper::recentUrl(filePath) };
    bool ok = false;
    QString error;
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_SetPermission",
                                     quint64(1), recentPerm,
                                     QFileDevice::Permissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner),
                                     &ok, &error));
    EXPECT_TRUE(ok);
    // The handler applied the owner read/write permission set (dfm-io may
    // additionally mirror owner bits onto user bits).
    const QFileDevice::Permissions perms = file.permissions();
    EXPECT_TRUE(perms & QFileDevice::ReadOwner);
    EXPECT_TRUE(perms & QFileDevice::WriteOwner);
}

// Non-recent inputs must make every hooked handler return false, which makes
// the whole hook traversal fail -> covers the negative branches of the
// EventHelper invocations.
TEST_F(RecentFollowEventsTest, HookRun_NonRecentInputs_TraversalFails)
{
    const QUrl plainFile { "file:///tmp/not_recent.txt" };
    QList<Global::ItemRoles> roles;
    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_workspace", "hook_Model_FetchCustomColumnRoles",
                                      plainFile, &roles));
    EXPECT_TRUE(roles.isEmpty());

    Global::TransparentStatus status {};
    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_workspace", "hook_Delegate_CheckTransparent",
                                      plainFile, &status));

    Qt::DropAction action { Qt::IgnoreAction };
    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_CheckDragDropAction",
                                      QList<QUrl> { plainFile }, plainFile, &action));

    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileDrop",
                                      QList<QUrl> { plainFile }, QUrl("trash:///")));

    QList<QVariantMap> crumbs;
    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_titlebar", "hook_Crumb_Seprate",
                                      plainFile, &crumbs));

    QString iconName;
    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_detailspace", "hook_Icon_Fetch",
                                      QUrl("recent:///not_root.txt"), &iconName));

    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_propertydialog", "hook_PropertyDialog_Disable",
                                      QUrl("recent:///not_root.txt")));

    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_MoveToTrash",
                                      quint64(1), QList<QUrl> { plainFile },
                                      AbstractJobHandler::JobFlags {}));
    EXPECT_FALSE(dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin",
                                      quint64(1), QList<QUrl> {}));
}

// Publish the GlobalEventType signals Recent::bindEvents() subscribed to so the
// EventHelper<void (RecentEventReceiver::*)(...)> dispatch instantiations run.
TEST_F(RecentFollowEventsTest, SignalPublish_BoundEvents_InvokeReceiverHandlers)
{
    const QList<QUrl> urls { QUrl("recent:///tmp/cut_src.txt") };
    const QList<QUrl> dests { QUrl("file:///tmp/cut_dst.txt") };
    const QMap<QUrl, QUrl> renamed { { QUrl("recent:///tmp/a.txt"), QUrl("file:///tmp/b.txt") } };

    EXPECT_EQ(reloadCount, 0);
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kCutFileResult, urls, dests, true, QString()));
    EXPECT_EQ(reloadCount, 1);

    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrashResult, urls, true, QString()));
    EXPECT_EQ(reloadCount, 2);

    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kDeleteFilesResult, urls, true, QString()));
    EXPECT_EQ(reloadCount, 3);

    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kRenameFileResult, quint64(1), renamed, true, QString()));
    EXPECT_EQ(reloadCount, 4);

    // Window url change on the recent scheme only posts a deferred slot push;
    // it must succeed without any registered slot consumer.
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, quint64(1),
                                             QUrl("recent:///")));
}

// Failed operations must not reload the recent cache: the receiver handlers
// short-circuit, covering the negative branches behind the dispatch helpers.
TEST_F(RecentFollowEventsTest, SignalPublish_FailedOperations_DoNotReload)
{
    const QList<QUrl> urls { QUrl("recent:///tmp/failed.txt") };

    EXPECT_EQ(reloadCount, 0);
    dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrashResult, urls, false, QString("boom"));
    EXPECT_EQ(reloadCount, 0);

    dpfSignalDispatcher->publish(GlobalEventType::kDeleteFilesResult, QList<QUrl> {}, true, QString());
    EXPECT_EQ(reloadCount, 0);

    dpfSignalDispatcher->publish(GlobalEventType::kCutFileResult, urls, QList<QUrl> {}, false, QString("boom"));
    EXPECT_EQ(reloadCount, 0);

    dpfSignalDispatcher->publish(GlobalEventType::kRenameFileResult, quint64(1),
                                 QMap<QUrl, QUrl> {}, true, QString());
    EXPECT_EQ(reloadCount, 0);
}
