// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "events/workspaceeventreceiver.h"
#include "utils/workspacehelper.h"
#include "views/fileview.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-framework/dpf.h>
#include <dfm-framework/event/event.h>

#include <QUrl>
#include <QVariant>
#include <QMap>
#include <QDir>
#include <QVariantList>
#include <QVariantMap>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_workspace;

namespace {
constexpr char kSpace[] { "dfmplugin_workspace" };

// All topics the receiver connects to must be registered on the framework's
// event converter, otherwise connect/push resolve to an invalid event type.
void registerWorkspaceTopics()
{
    static bool registered = false;
    if (registered)
        return;
    registered = true;

    auto *event = dpfEvent;
    const char *slotTopics[] = {
        "slot_RegisterFileView",
        "slot_RegisterMenuScene",
        "slot_FindMenuScene",
        "slot_RegisterCustomTopWidget",
        "slot_ShowViewHint",
        "slot_RegisterGroupStrategy",
        "slot_RegisteredGroupStrategies",
        "slot_GetCustomTopWidgetVisible",
        "slot_ShowCustomTopWidget",
        "slot_CheckSchemeViewIsFileView",
        "slot_RefreshDir",
        "slot_RegisterFocusFileViewDisabled",
        "slot_View_SetCustomViewProperty",
        "slot_View_GetVisualGeometry",
        "slot_View_GetViewItemRect",
        "slot_View_GetCurrentViewMode",
        "slot_View_GetDefaultViewMode",
        "slot_View_GetSelectedUrls",
        "slot_View_SelectFiles",
        "slot_View_SelectAll",
        "slot_View_ReverseSelect",
        "slot_View_SetSelectionMode",
        "slot_View_SetEnabledSelectionModes",
        "slot_View_SetDragEnabled",
        "slot_View_SetDragDropMode",
        "slot_View_SetReadOnly",
        "slot_View_SetFilter",
        "slot_View_GetFilter",
        "slot_View_ClosePersistentEditor",
        "slot_View_SetAlwaysOpenInCurrentWindow",
        "slot_Model_SetCustomFilterData",
        "slot_Model_SetCustomFilterCallback",
        "slot_Model_RegisterRoutePrehandle",
        "slot_Model_FileUpdate",
        "slot_Model_SetNameFilter",
        "slot_Model_GetNameFilter",
        "slot_Model_CurrentSortRole",
        "slot_Model_ColumnDisplayName",
        "slot_Model_ColumnRoles",
        "slot_Model_SetSort",
        "slot_Model_CurrentGroupStrategy",
        "slot_Model_SetGroup",
        "slot_Model_RegisterDataCache",
        "slot_View_AboutToChangeViewWidth",
        "slot_View_GetColumnWidth",
        "slot_Model_RegisterLoadStrategy",
        "slot_Model_GetCurrentBusy",
    };
    for (const char *topic : slotTopics)
        event->registerEventType(EventStratege::kSlot, kSpace, topic);

    event->registerEventType(EventStratege::kSignal, "dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged");
    event->registerEventType(EventStratege::kSignal, "dfmplugin_titlebar", "signal_Tab_Created");
    event->registerEventType(EventStratege::kSignal, "dfmplugin_titlebar", "signal_Tab_Removed");
    event->registerEventType(EventStratege::kSignal, "dfmplugin_titlebar", "signal_Tab_Changed");
    event->registerEventType(EventStratege::kSignal, kSpace, "signal_View_HeaderViewSectionChanged");
}
}   // namespace

class WorkspaceEventFlowTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        registerWorkspaceTopics();
        WorkspaceEventReceiver::instance()->initConnection();
        channel = dpfSlotChannel;

        stub.set_lamda(ADDR(dfmbase::Application, appAttribute),
                       [](dfmbase::Application::ApplicationAttribute) { return QVariant(1); });
        stub.set_lamda(ADDR(dfmbase::Application, setAppAttribute),
                       [](dfmbase::Application::ApplicationAttribute, const QVariant &) {});
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
    EventChannelManager *channel = nullptr;
};

// --- registration slots ---

TEST_F(WorkspaceEventFlowTest, Push_RegisterFileView_CheckSchemeReturnsTrue)
{
    QVariant voidRet = channel->push(kSpace, "slot_RegisterFileView", QString("flowfile"));
    EXPECT_FALSE(voidRet.isValid());

    QVariant ret = channel->push(kSpace, "slot_CheckSchemeViewIsFileView", QString("flowfile"));
    ASSERT_TRUE(ret.isValid());
    EXPECT_TRUE(ret.toBool());
}

TEST_F(WorkspaceEventFlowTest, Push_RegisterMenuScene_FindMenuSceneReturnsScene)
{
    channel->push(kSpace, "slot_RegisterMenuScene", QString("flowfile"), QString("FlowScene"));

    QVariant ret = channel->push(kSpace, "slot_FindMenuScene", QString("flowfile"));
    ASSERT_TRUE(ret.isValid());
    EXPECT_EQ(ret.toString(), QString("FlowScene"));
    EXPECT_EQ(channel->push(kSpace, "slot_FindMenuScene", QString("unregistered")).toString(), QString(""));
}

TEST_F(WorkspaceEventFlowTest, Push_RegisterFocusFileViewDisabled_DoesNotCrash)
{
    QVariant ret = channel->push(kSpace, "slot_RegisterFocusFileViewDisabled", QString("flowfile"));
    EXPECT_FALSE(ret.isValid());
    EXPECT_NE(WorkspaceHelper::instance(), nullptr);
}

TEST_F(WorkspaceEventFlowTest, Push_RegisterCustomTopWidget_GetVisibleReturnsFalse)
{
    QVariantMap dataMap;
    dataMap.insert("scheme", "flowscheme");
    channel->push(kSpace, "slot_RegisterCustomTopWidget", dataMap);

    QVariant ret = channel->push(kSpace, "slot_GetCustomTopWidgetVisible", quint64(1), QString("flowscheme"));
    ASSERT_TRUE(ret.isValid());
    EXPECT_FALSE(ret.toBool());
}

TEST_F(WorkspaceEventFlowTest, Push_ShowViewHint_NoWorkspace_ReturnsNull)
{
    QVariantMap content;
    QVariant ret = channel->push(kSpace, "slot_ShowViewHint", quint64(1), content);
    ASSERT_TRUE(ret.isValid());
    EXPECT_TRUE(ret.value<QObject *>() == nullptr);
}

TEST_F(WorkspaceEventFlowTest, Push_RegisteredGroupStrategies_ReturnsVariantList)
{
    QVariant ret = channel->push(kSpace, "slot_RegisteredGroupStrategies", QString());
    EXPECT_TRUE(ret.isValid());
    EXPECT_TRUE(ret.canConvert<QVariantList>());
}

TEST_F(WorkspaceEventFlowTest, Push_RegisterGroupStrategy_EmptyNameIsRejected)
{
    QVariantMap dataMap;   // no name: handler rejects and returns nothing
    QVariant ret = channel->push(kSpace, "slot_RegisterGroupStrategy", dataMap);
    EXPECT_FALSE(ret.isValid());
}

TEST_F(WorkspaceEventFlowTest, Push_RegisterGroupStrategy_WithFactoryAndName_Registers)
{
    StrategyFactory factory = [](QObject *) -> DFMBASE_NAMESPACE::AbstractGroupStrategy * { return nullptr; };
    QVariantMap dataMap;
    dataMap.insert(PropertyKey::kGroupStrategyName, QString("FlowTimeStrategy"));
    dataMap.insert(PropertyKey::kGroupStrategyDisplayName, QString("Flow time"));
    dataMap.insert(PropertyKey::kGroupStrategyFactory, QVariant::fromValue(factory));

    QVariant ret = channel->push(kSpace, "slot_RegisterGroupStrategy", dataMap);
    EXPECT_FALSE(ret.isValid());

    QVariantList registered = channel->push(kSpace, "slot_RegisteredGroupStrategies", QString("flowfile"))
                                      .value<QVariantList>();
    bool found = false;
    for (const QVariant &entry : registered)
        found = found || entry.toMap().value("name").toString() == QString("FlowTimeStrategy");
    EXPECT_TRUE(found);
    EXPECT_GE(registered.size(), 1);
}

// --- view getter slots (no window registered) ---

TEST_F(WorkspaceEventFlowTest, Push_ViewGetters_UnknownWindow_ReturnDefaults)
{
    EXPECT_EQ(channel->push(kSpace, "slot_View_GetCurrentViewMode", quint64(42)).value<ViewMode>(),
              ViewMode::kNoneMode);
    EXPECT_EQ(channel->push(kSpace, "slot_View_GetVisualGeometry", quint64(42)).toRectF(),
              QRectF(0, 0, 0, 0));
    EXPECT_TRUE(channel->push(kSpace, "slot_View_GetSelectedUrls", quint64(42)).value<QList<QUrl>>().isEmpty());
}

TEST_F(WorkspaceEventFlowTest, Push_ViewGetViewItemRect_UnknownWindow_ReturnsEmptyRect)
{
    QVariant ret = channel->push(kSpace, "slot_View_GetViewItemRect", quint64(42),
                                 QUrl::fromLocalFile("/tmp/flow/a.txt"),
                                 QVariant::fromValue(ItemRoles::kItemIconRole));
    ASSERT_TRUE(ret.isValid());
    EXPECT_EQ(ret.toRectF(), QRectF(0, 0, 0, 0));
}

TEST_F(WorkspaceEventFlowTest, Push_ModelGetters_UnknownWindow_ReturnDefaults)
{
    EXPECT_FALSE(channel->push(kSpace, "slot_Model_GetCurrentBusy", quint64(42)).toBool());
    EXPECT_EQ(channel->push(kSpace, "slot_Model_CurrentSortRole", quint64(42)).value<ItemRoles>(),
              ItemRoles::kItemUnknowRole);
    EXPECT_TRUE(channel->push(kSpace, "slot_Model_ColumnRoles", quint64(42)).value<QList<ItemRoles>>().isEmpty());
    EXPECT_EQ(channel->push(kSpace, "slot_Model_CurrentGroupStrategy", quint64(42)).toString(), QString(""));
}

TEST_F(WorkspaceEventFlowTest, Push_ViewGetFilter_UnknownWindow_ReturnsNoFilter)
{
    QVariant ret = channel->push(kSpace, "slot_View_GetFilter", quint64(42));
    ASSERT_TRUE(ret.isValid());
    EXPECT_EQ(ret.toInt(), static_cast<int>(QDir::NoFilter));
}

TEST_F(WorkspaceEventFlowTest, Push_ModelGetNameFilter_UnknownWindow_ReturnsEmpty)
{
    QVariant ret = channel->push(kSpace, "slot_Model_GetNameFilter", quint64(42));
    ASSERT_TRUE(ret.isValid());
    EXPECT_TRUE(ret.toStringList().isEmpty());
}

TEST_F(WorkspaceEventFlowTest, Push_ModelColumnDisplayName_UnknownWindow_ReturnsEmpty)
{
    QVariant ret = channel->push(kSpace, "slot_Model_ColumnDisplayName", quint64(42),
                                 QVariant::fromValue(ItemRoles::kItemFileDisplayNameRole));
    EXPECT_TRUE(ret.isValid());
    EXPECT_TRUE(ret.toString().isEmpty());
}

TEST_F(WorkspaceEventFlowTest, Push_ViewGetColumnWidth_UnknownWindow_ReturnsZero)
{
    QVariant ret = channel->push(kSpace, "slot_View_GetColumnWidth", quint64(42),
                                 QVariant::fromValue(ItemRoles::kItemFileSizeRole));
    ASSERT_TRUE(ret.isValid());
    EXPECT_EQ(ret.toInt(), 0);
}

TEST_F(WorkspaceEventFlowTest, Push_ViewGetDefaultViewMode_FallsBackToValidMode)
{
    QVariant ret = channel->push(kSpace, "slot_View_GetDefaultViewMode", QString("flowfile"));
    ASSERT_TRUE(ret.isValid());
    ViewMode mode = ret.value<ViewMode>();
    EXPECT_GE(static_cast<int>(mode), static_cast<int>(ViewMode::kIconMode));
    EXPECT_LE(static_cast<int>(mode), static_cast<int>(ViewMode::kAllViewMode));
}

// --- setter slots on unknown windows (must not crash, return nothing) ---

TEST_F(WorkspaceEventFlowTest, Push_ViewSetters_UnknownWindow_ReturnVoid)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/flow/a.txt") };
    EXPECT_FALSE(channel->push(kSpace, "slot_View_SelectFiles", quint64(42), urls).isValid());
    EXPECT_FALSE(channel->push(kSpace, "slot_View_SelectAll", quint64(42)).isValid());
    EXPECT_FALSE(channel->push(kSpace, "slot_View_ReverseSelect", quint64(42)).isValid());
    EXPECT_FALSE(channel->push(kSpace, "slot_View_SetAlwaysOpenInCurrentWindow", quint64(42)).isValid());
    EXPECT_FALSE(channel->push(kSpace, "slot_View_ClosePersistentEditor", quint64(42)).isValid());
    EXPECT_NE(WorkspaceHelper::instance(), nullptr);
}

TEST_F(WorkspaceEventFlowTest, Push_ViewModeSetters_UnknownWindow_ReturnVoid)
{
    EXPECT_FALSE(channel->push(kSpace, "slot_View_SetSelectionMode", quint64(42),
                               QVariant::fromValue(QAbstractItemView::SingleSelection)).isValid());
    QList<int> modes = { int(QAbstractItemView::ExtendedSelection) };
    EXPECT_FALSE(channel->push(kSpace, "slot_View_SetEnabledSelectionModes", quint64(42), modes).isValid());
    EXPECT_FALSE(channel->push(kSpace, "slot_View_SetDragEnabled", quint64(42), true).isValid());
    EXPECT_FALSE(channel->push(kSpace, "slot_View_SetDragDropMode", quint64(42),
                               QVariant::fromValue(QAbstractItemView::DragDrop)).isValid());
    EXPECT_FALSE(channel->push(kSpace, "slot_View_SetReadOnly", quint64(42), true).isValid());
}

TEST_F(WorkspaceEventFlowTest, Push_ModelSetters_UnknownWindow_ReturnVoid)
{
    EXPECT_FALSE(channel->push(kSpace, "slot_Model_SetSort", quint64(42),
                               QVariant::fromValue(ItemRoles::kItemFileSizeRole)).isValid());
    EXPECT_FALSE(channel->push(kSpace, "slot_Model_SetGroup", quint64(42), QString("TimeModified")).isValid());
    EXPECT_FALSE(channel->push(kSpace, "slot_Model_SetCustomFilterData", quint64(42),
                               QUrl::fromLocalFile("/tmp/flow"), QVariant("kw")).isValid());
    EXPECT_FALSE(channel->push(kSpace, "slot_Model_SetCustomFilterCallback", quint64(42),
                               QUrl::fromLocalFile("/tmp/flow"), QVariant()).isValid());
    EXPECT_FALSE(channel->push(kSpace, "slot_Model_RegisterDataCache", QString("flowfile")).isValid());
}

TEST_F(WorkspaceEventFlowTest, Push_ModelFileUpdate_AndFilterSetters_ReturnVoid)
{
    EXPECT_FALSE(channel->push(kSpace, "slot_Model_FileUpdate", QUrl::fromLocalFile("/tmp/flow/a.txt")).isValid());
    EXPECT_FALSE(channel->push(kSpace, "slot_View_SetFilter", quint64(42),
                               QVariant::fromValue(int(QDir::Files))).isValid());
    EXPECT_FALSE(channel->push(kSpace, "slot_Model_SetNameFilter", quint64(42), QStringList() << "*.txt").isValid());
    EXPECT_FALSE(channel->push(kSpace, "slot_Model_RegisterLoadStrategy", QString("flowfile"),
                               QVariant::fromValue(DirectoryLoadStrategy::kCreateNew)).isValid());
}

TEST_F(WorkspaceEventFlowTest, Push_ViewAboutToChangeViewWidth_AndCustomProperty_ReturnVoid)
{
    EXPECT_FALSE(channel->push(kSpace, "slot_View_AboutToChangeViewWidth", quint64(42), 30).isValid());

    QVariantMap properties;
    properties.insert("allowChangeListHeight", true);
    EXPECT_FALSE(channel->push(kSpace, "slot_View_SetCustomViewProperty", QString("flowfile"), properties).isValid());
}

TEST_F(WorkspaceEventFlowTest, Push_ShowCustomTopWidget_UnknownWindow_ReturnVoid)
{
    EXPECT_FALSE(channel->push(kSpace, "slot_ShowCustomTopWidget", quint64(42), QString("flowscheme"), true).isValid());
    EXPECT_NE(WorkspaceHelper::instance(), nullptr);
}

TEST_F(WorkspaceEventFlowTest, Push_RegisterRoutePrehandle_ReturnsRegistrationResult)
{
    // QVariant cannot carry the std::function, handler receives an empty one
    QVariant ret = channel->push(kSpace, "slot_Model_RegisterRoutePrehandle", QString("flowfile"), QVariant());
    ASSERT_TRUE(ret.isValid());
    EXPECT_TRUE(ret.toBool());
}

TEST_F(WorkspaceEventFlowTest, Push_RefreshDir_EmptyWorkspaces_ReturnVoid)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/flow") };
    EXPECT_FALSE(channel->push(kSpace, "slot_RefreshDir", urls).isValid());
}

// --- signal dispatch through EventDispatcherManager ---

TEST_F(WorkspaceEventFlowTest, Publish_TrashStateChanged_InvokesHelperSignal)
{
    EXPECT_TRUE(dpfSignalDispatcher->publish("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged"));
    EXPECT_NE(WorkspaceHelper::instance(), nullptr);
}

TEST_F(WorkspaceEventFlowTest, Publish_TabSignals_InvokedWithoutCrash)
{
    EXPECT_TRUE(dpfSignalDispatcher->publish("dfmplugin_titlebar", "signal_Tab_Created", quint64(1), QString("tab-1")));
    EXPECT_TRUE(dpfSignalDispatcher->publish("dfmplugin_titlebar", "signal_Tab_Removed", quint64(1), QString("tab-1"), QString("tab-2")));
    EXPECT_TRUE(dpfSignalDispatcher->publish("dfmplugin_titlebar", "signal_Tab_Changed", quint64(1), QString("tab-2")));
}

TEST_F(WorkspaceEventFlowTest, Publish_GlobalEventTypes_InvokedWithoutCrash)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/flow/a.txt") };
    QMap<QUrl, QUrl> renamed;
    renamed.insert(QUrl::fromLocalFile("/tmp/flow/a.txt"), QUrl::fromLocalFile("/tmp/flow/b.txt"));

    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kSwitchViewMode, quint64(1), static_cast<int>(ViewMode::kListMode)));
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kCopyResult, urls, urls, true, QString()));
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kCutFileResult, urls, urls, false, QString("err")));
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrashResult, urls, true, QString()));
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kDeleteFilesResult, urls, false, QString("err")));
    EXPECT_TRUE(dpfSignalDispatcher->publish(GlobalEventType::kRenameFileResult, quint64(1), renamed, false, QString("err")));
}

TEST_F(WorkspaceEventFlowTest, Publish_HeaderViewSectionChanged_InvokesFileViewSlot)
{
    FileView view(QUrl::fromLocalFile("/tmp/flow"));
    view.setViewMode(Global::ViewMode::kListMode);

    EXPECT_NO_FATAL_FAILURE(view.onHeaderSectionMoved(0, 0, 1));   // publishes the signal
    EXPECT_TRUE(dpfSignalDispatcher->publish(kSpace, "signal_View_HeaderViewSectionChanged", view.rootUrl()));
    EXPECT_EQ(view.currentViewMode(), Global::ViewMode::kListMode);
}

TEST_F(WorkspaceEventFlowTest, Unsubscribe_TrashAndTabSignals_RemoveHandlers)
{
    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged",
                                                 WorkspaceHelper::instance(), &WorkspaceHelper::trashStateChanged));
    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe("dfmplugin_titlebar", "signal_Tab_Created",
                                                 WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleTabCreated));
    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe("dfmplugin_titlebar", "signal_Tab_Removed",
                                                 WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleTabRemoved));
    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe("dfmplugin_titlebar", "signal_Tab_Changed",
                                                 WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleTabChanged));

    // handlers removed, but publish still dispatches without crashing
    EXPECT_TRUE(dpfSignalDispatcher->publish("dfmplugin_titlebar", "signal_Tab_Created", quint64(9), QString("tab-9")));
}
