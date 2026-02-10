// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "events/titlebareventcaller.h"
#include "utils/titlebarhelper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>
#include <QWidget>
#include <QUrl>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPF_USE_NAMESPACE

class TitleBarEventCallerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        widget = new QWidget();
        stub.clear();

        // Stub windowId to return a valid ID by default
        stub.set_lamda(&TitleBarHelper::windowId, [](QWidget *) {
            __DBG_STUB_INVOKE__
            return static_cast<quint64>(12345);
        });
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

    QWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(TitleBarEventCallerTest, SendViewMode_IconMode_SignalPublished)
{
    bool signalPublished = false;
    Global::ViewMode capturedMode = Global::ViewMode::kIconMode;
    quint64 capturedId = 0;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, int &&);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType topic, quint64 id, int mode) {
                       __DBG_STUB_INVOKE__
                       if (topic == DFMBASE_NAMESPACE::GlobalEventType::kSwitchViewMode) {
                           signalPublished = true;
                           capturedId = id;
                           capturedMode = static_cast<Global::ViewMode>(mode);
                       }
                       return true;
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendViewMode(widget, Global::ViewMode::kIconMode);
    EXPECT_TRUE(signalPublished);
    EXPECT_EQ(capturedId, 12345);
    EXPECT_EQ(capturedMode, Global::ViewMode::kIconMode);
}

TEST_F(TitleBarEventCallerTest, SendDetailViewState_Checked_SlotCalled)
{
    bool slotCalled = false;
    bool capturedChecked = false;
    quint64 capturedId = 0;

    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, quint64, bool &);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [&](EventChannelManager *, const QString &space, const QString &topic, quint64 id, bool checked) {
                       __DBG_STUB_INVOKE__
                       if (space == "dfmplugin_detailspace" && topic == "slot_DetailView_Show") {
                           slotCalled = true;
                           capturedId = id;
                           capturedChecked = checked;
                       }
                       return QVariant();
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendDetailViewState(widget, true);
    EXPECT_TRUE(slotCalled);
    EXPECT_EQ(capturedId, 12345);
    EXPECT_TRUE(capturedChecked);
}

TEST_F(TitleBarEventCallerTest, SendCd_ValidUrl_SignalPublished)
{
    QUrl testUrl("file:///home/test");
    bool signalPublished = false;
    QUrl capturedUrl;
    quint64 capturedId = 0;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType topic, quint64 id, const QUrl &url) {
                       __DBG_STUB_INVOKE__
                       if (topic == DFMBASE_NAMESPACE::GlobalEventType::kChangeCurrentUrl) {
                           signalPublished = true;
                           capturedId = id;
                           capturedUrl = url;
                       }
                       return true;
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendCd(widget, testUrl);
    EXPECT_TRUE(signalPublished);
    EXPECT_EQ(capturedId, 12345);
    EXPECT_EQ(capturedUrl, testUrl);
}

TEST_F(TitleBarEventCallerTest, SendCd_InvalidUrl_NoSignalPublished)
{
    QUrl invalidUrl;
    bool signalPublished = false;

    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendCd(widget, invalidUrl);
    EXPECT_FALSE(signalPublished);
}

TEST_F(TitleBarEventCallerTest, SendChangeCurrentUrl_ValidUrl_SignalPublished)
{
    QUrl testUrl("file:///home/test");
    bool signalPublished = false;
    QUrl capturedUrl;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType topic, quint64 id, const QUrl &url) {
                       __DBG_STUB_INVOKE__
                       if (topic == DFMBASE_NAMESPACE::GlobalEventType::kChangeCurrentUrl) {
                           signalPublished = true;
                           capturedUrl = url;
                       }
                       return true;
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendChangeCurrentUrl(widget, testUrl);
    EXPECT_TRUE(signalPublished);
    EXPECT_EQ(capturedUrl, testUrl);
}

TEST_F(TitleBarEventCallerTest, SendChangeCurrentUrl_InvalidUrl_NoSignalPublished)
{
    QUrl invalidUrl;
    bool signalPublished = false;

    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendChangeCurrentUrl(widget, invalidUrl);
    EXPECT_FALSE(signalPublished);
}

TEST_F(TitleBarEventCallerTest, SendOpenFile_ValidUrl_SignalPublished)
{
    QUrl testUrl("file:///home/test/file.txt");
    bool signalPublished = false;
    QList<QUrl> capturedUrls;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, QList<QUrl> &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType topic, quint64 id, QList<QUrl> &urls) {
                       __DBG_STUB_INVOKE__
                       if (topic == DFMBASE_NAMESPACE::GlobalEventType::kOpenFiles) {
                           signalPublished = true;
                           capturedUrls = urls;
                       }
                       return true;
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendOpenFile(widget, testUrl);
    EXPECT_TRUE(signalPublished);
    EXPECT_EQ(capturedUrls.size(), 1);
    EXPECT_EQ(capturedUrls[0], testUrl);
}

TEST_F(TitleBarEventCallerTest, SendOpenWindow_ValidUrl_SignalPublished)
{
    QUrl testUrl("file:///home/test");
    bool signalPublished = false;
    QUrl capturedUrl;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, QUrl);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType topic, QUrl url) {
                       __DBG_STUB_INVOKE__
                       if (topic == DFMBASE_NAMESPACE::GlobalEventType::kOpenNewWindow) {
                           signalPublished = true;
                           capturedUrl = url;
                       }
                       return true;
                   });

    TitleBarEventCaller::sendOpenWindow(testUrl);
    EXPECT_TRUE(signalPublished);
    EXPECT_EQ(capturedUrl, testUrl);
}

TEST_F(TitleBarEventCallerTest, SendOpenTab_ValidWindowIdAndUrl_SignalPublished)
{
    quint64 windowId = 54321;
    QUrl testUrl("file:///home/test");
    bool signalPublished = false;
    quint64 capturedId = 0;
    QUrl capturedUrl;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType topic, quint64 id, const QUrl &url) {
                       __DBG_STUB_INVOKE__
                       if (topic == DFMBASE_NAMESPACE::GlobalEventType::kOpenNewTab) {
                           signalPublished = true;
                           capturedId = id;
                           capturedUrl = url;
                       }
                       return true;
                   });

    TitleBarEventCaller::sendOpenTab(windowId, testUrl);
    EXPECT_TRUE(signalPublished);
    EXPECT_EQ(capturedId, windowId);
    EXPECT_EQ(capturedUrl, testUrl);
}

TEST_F(TitleBarEventCallerTest, SendSearch_ValidKeyword_SignalPublished)
{
    QString keyword = "test search";
    bool signalPublished = false;
    QString capturedKeyword;
    quint64 capturedId = 0;

    typedef bool (EventDispatcherManager::*PublishFunc)(const QString &, const QString &, quint64, const QString &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, const QString &space, const QString &topic, quint64 id, const QString &key) {
                       __DBG_STUB_INVOKE__
                       if (topic == "signal_Search_Start") {
                           signalPublished = true;
                           capturedId = id;
                           capturedKeyword = key;
                       }
                       return true;
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendSearch(widget, keyword);
    EXPECT_TRUE(signalPublished);
    EXPECT_EQ(capturedId, 12345);
    EXPECT_EQ(capturedKeyword, keyword);
}

TEST_F(TitleBarEventCallerTest, SendStopSearch_Called_SignalPublished)
{
    bool signalPublished = false;
    quint64 capturedId = 0;

    typedef bool (EventDispatcherManager::*PublishFunc)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, const QString &space, const QString &topic, quint64 id) {
                       __DBG_STUB_INVOKE__
                       if (topic == "signal_Search_Stop") {
                           signalPublished = true;
                           capturedId = id;
                       }
                       return true;
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendStopSearch(widget);
    EXPECT_TRUE(signalPublished);
    EXPECT_EQ(capturedId, 12345);
}

TEST_F(TitleBarEventCallerTest, SendShowFilterView_Visible_SignalPublished)
{
    bool signalPublished = false;
    bool capturedVisible = false;

    typedef bool (EventDispatcherManager::*PublishFunc)(const QString &, const QString &, quint64, bool &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, const QString &space, const QString &topic, quint64 id, bool visible) {
                       __DBG_STUB_INVOKE__
                       if (topic == "signal_FilterView_Show") {
                           signalPublished = true;
                           capturedVisible = visible;
                       }
                       return true;
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendShowFilterView(widget, true);
    EXPECT_TRUE(signalPublished);
    EXPECT_TRUE(capturedVisible);
}

TEST_F(TitleBarEventCallerTest, SendCheckAddressInputStr_ValidString_SignalPublished)
{
    QString testStr = "file:///home/test";
    bool signalPublished = false;

    typedef bool (EventDispatcherManager::*PublishFunc)(const QString &, const QString &, quint64, QString *&);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, const QString &space, const QString &topic, quint64 id, QString *) {
                       __DBG_STUB_INVOKE__
                       if (topic == "signal_InputAdddressStr_Check") {
                           signalPublished = true;
                       }
                       return true;
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendCheckAddressInputStr(widget, &testStr);
    EXPECT_TRUE(signalPublished);
}

TEST_F(TitleBarEventCallerTest, SendTabChanged_ValidUniqueId_SignalPublished)
{
    QString uniqueId = "tab123";
    bool signalPublished = false;
    QString capturedId;
    quint64 capturedWindowId = 0;

    typedef bool (EventDispatcherManager::*PublishFunc)(const QString &, const QString &, quint64, const QString &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, const QString &space, const QString &topic, quint64 id, const QString &uid) {
                       __DBG_STUB_INVOKE__
                       if (topic == "signal_Tab_Changed") {
                           signalPublished = true;
                           capturedWindowId = id;
                           capturedId = uid;
                       }
                       return true;
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendTabChanged(widget, uniqueId);
    EXPECT_TRUE(signalPublished);
    EXPECT_EQ(capturedWindowId, 12345);
    EXPECT_EQ(capturedId, uniqueId);
}

TEST_F(TitleBarEventCallerTest, SendTabCreated_ValidUniqueId_SignalPublished)
{
    QString uniqueId = "tab456";
    bool signalPublished = false;
    QString capturedId;

    typedef bool (EventDispatcherManager::*PublishFunc)(const QString &, const QString &, quint64, const QString &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, const QString &space, const QString &topic, quint64 id, const QString &uid) {
                       __DBG_STUB_INVOKE__
                       if (topic == "signal_Tab_Created") {
                           signalPublished = true;
                           capturedId = uid;
                       }
                       return true;
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendTabCreated(widget, uniqueId);
    EXPECT_TRUE(signalPublished);
    EXPECT_EQ(capturedId, uniqueId);
}

TEST_F(TitleBarEventCallerTest, SendTabRemoved_ValidIds_SignalPublished)
{
    QString removedId = "tab1";
    QString nextId = "tab2";
    bool signalPublished = false;
    QString capturedRemovedId;
    QString capturedNextId;

    typedef bool (EventDispatcherManager::*PublishFunc)(const QString &, const QString &, quint64, const QString &, const QString &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, const QString &space, const QString &topic, quint64 id, const QString &ruid, const QString &nuid) {
                       __DBG_STUB_INVOKE__
                       if (topic == "signal_Tab_Removed") {
                           signalPublished = true;
                           capturedRemovedId = ruid;
                           capturedNextId = nuid;
                       }
                       return true;
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendTabRemoved(widget, removedId, nextId);
    EXPECT_TRUE(signalPublished);
    EXPECT_EQ(capturedRemovedId, removedId);
    EXPECT_EQ(capturedNextId, nextId);
}

TEST_F(TitleBarEventCallerTest, SendColumnDisplyName_ValidRole_ReturnsName)
{
    QString expectedName = "Name";
    ItemRoles role = ItemRoles::kItemFileDisplayNameRole;

    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, quint64, Global::ItemRoles &);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [&](EventChannelManager *, const QString &space, const QString &topic, quint64 id, Global::ItemRoles) {
                       __DBG_STUB_INVOKE__
                       if (space == "dfmplugin_workspace" && topic == "slot_Model_ColumnDisplayName") {
                           return QVariant(expectedName);
                       }
                       return QVariant();
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    QString name = TitleBarEventCaller::sendColumnDisplyName(widget, role);
    EXPECT_EQ(name, expectedName);
}

TEST_F(TitleBarEventCallerTest, SendColumnRoles_Called_ReturnsRoleList)
{
    QList<ItemRoles> expectedRoles;
    expectedRoles << ItemRoles::kItemFileDisplayNameRole << ItemRoles::kItemFileSizeRole;

    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [&](EventChannelManager *, const QString &space, const QString &topic, quint64 id) {
                       __DBG_STUB_INVOKE__
                       if (space == "dfmplugin_workspace" && topic == "slot_Model_ColumnRoles") {
                           return QVariant::fromValue(expectedRoles);
                       }
                       return QVariant();
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    QList<ItemRoles> roles = TitleBarEventCaller::sendColumnRoles(widget);
    EXPECT_EQ(roles.size(), 2);
    EXPECT_EQ(roles, expectedRoles);
}

TEST_F(TitleBarEventCallerTest, SendGetDefualtViewMode_FileScheme_ReturnsIconMode)
{
    ViewMode expectedMode = ViewMode::kIconMode;

    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, QString);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [&](EventChannelManager *, const QString &space, const QString &topic, QString) {
                       __DBG_STUB_INVOKE__
                       if (space == "dfmplugin_workspace" && topic == "slot_View_GetDefaultViewMode") {
                           return QVariant(static_cast<int>(expectedMode));
                       }
                       return QVariant();
                   });

    ViewMode mode = TitleBarEventCaller::sendGetDefualtViewMode("file");
    EXPECT_EQ(mode, expectedMode);
}

TEST_F(TitleBarEventCallerTest, SendCurrentSortRole_Called_ReturnsRole)
{
    ItemRoles expectedRole = ItemRoles::kItemFileDisplayNameRole;

    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [&](EventChannelManager *, const QString &space, const QString &topic, quint64 id) {
                       __DBG_STUB_INVOKE__
                       if (space == "dfmplugin_workspace" && topic == "slot_Model_CurrentSortRole") {
                           return QVariant(static_cast<int>(expectedRole));
                       }
                       return QVariant();
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    ItemRoles role = TitleBarEventCaller::sendCurrentSortRole(widget);
    EXPECT_EQ(role, expectedRole);
}

TEST_F(TitleBarEventCallerTest, SendSetSort_ValidRole_SlotCalled)
{
    bool slotCalled = false;
    ItemRoles capturedRole = ItemRoles::kItemFileDisplayNameRole;

    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, quint64, Global::ItemRoles &);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [&](EventChannelManager *, const QString &space, const QString &topic, quint64 id, Global::ItemRoles role) {
                       __DBG_STUB_INVOKE__
                       if (space == "dfmplugin_workspace" && topic == "slot_Model_SetSort") {
                           slotCalled = true;
                           capturedRole = role;
                       }
                       return QVariant();
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendSetSort(widget, ItemRoles::kItemFileSizeRole);
    EXPECT_TRUE(slotCalled);
    EXPECT_EQ(capturedRole, ItemRoles::kItemFileSizeRole);
}

TEST_F(TitleBarEventCallerTest, SendCurrentGroupRoleStrategy_Called_ReturnsStrategy)
{
    QString expectedStrategy = "type";

    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [&](EventChannelManager *, const QString &space, const QString &topic, quint64 id) {
                       __DBG_STUB_INVOKE__
                       if (space == "dfmplugin_workspace" && topic == "slot_Model_CurrentGroupStrategy") {
                           return QVariant(expectedStrategy);
                       }
                       return QVariant();
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    QString strategy = TitleBarEventCaller::sendCurrentGroupRoleStrategy(widget);
    EXPECT_EQ(strategy, expectedStrategy);
}

TEST_F(TitleBarEventCallerTest, SendSetGroupStrategy_ValidStrategy_SlotCalled)
{
    bool slotCalled = false;
    QString capturedStrategy;

    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, quint64, const QString &);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [&](EventChannelManager *, const QString &space, const QString &topic, quint64 id, const QString &strategy) {
                       __DBG_STUB_INVOKE__
                       if (space == "dfmplugin_workspace" && topic == "slot_Model_SetGroup") {
                           slotCalled = true;
                           capturedStrategy = strategy;
                       }
                       return QVariant();
                   });
    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 12345; });

    TitleBarEventCaller::sendSetGroupStrategy(widget, "time");
    EXPECT_TRUE(slotCalled);
    EXPECT_EQ(capturedStrategy, "time");
}
