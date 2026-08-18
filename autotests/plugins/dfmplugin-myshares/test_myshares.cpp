// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "myshares.h"
#include "utils/shareutils.h"
#include "utils/sharefilehelper.h"
#include "fileinfo/sharefileinfo.h"
#include "iterator/shareiterator.h"
#include "watcher/sharewatcher.h"
#include "menu/mysharemenuscene.h"
#include "events/shareeventscaller.h"
#include "events/shareeventhelper.h"
#include "dfmplugin_myshares_global.h"
#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/filemanagerwindow.h>

#include <dfm-framework/dpf.h>
#include <dfm-framework/lifecycle/lifecycle.h>
#include <dfm-framework/listener/listener.h>

#include <QUrl>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QList>
#include <QVariantMap>
#include <QFileInfo>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

// exact template instantiations used by myshares.cpp (T by value, pack by reference kind)
using PushNoArg = QVariant (EventChannelManager::*)(const QString &, const QString &);   // slot_Share_AllShareInfos
using PushQString = QVariant (EventChannelManager::*)(const QString &, const QString &, QString);   // slot_RegisterFileView / menuSceneContains
using PushQStringQStrRRef = QVariant (EventChannelManager::*)(const QString &, const QString &, QString, QString &&);   // slot_RegisterMenuScene
using PushQStringQStrCRef = QVariant (EventChannelManager::*)(const QString &, const QString &, QString, const QString &);   // slot_MenuScene_Bind
using PushQUrl = QVariant (EventChannelManager::*)(const QString &, const QString &, QUrl);   // slot_Item_Remove
using PushSidebarInsert = QVariant (EventChannelManager::*)(const QString &, const QString &, int, QUrl &&, QVariantMap &);   // slot_Item_Insert
using PushQStringMapRef = QVariant (EventChannelManager::*)(const QString &, const QString &, QString, QVariantMap &);   // slot_Custom_Register / slot_View_SetCustomViewProperty
using PushCreatorRef = QVariant (EventChannelManager::*)(const QString &, const QString &, QString, AbstractSceneCreator *&);   // slot_MenuScene_RegisterScene
using SubscribeMyShares = bool (EventDispatcherManager::*)(const QString &, const QString &, MyShares *, void (MyShares::*)(const QString &));
using PublishMenuLog = bool (EventDispatcherManager::*)(const QString &, const QString &, QString, QList<QUrl> &);

class UT_MyShares : public testing::Test
{
public:
    virtual void SetUp() override { }
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
    MyShares ins;
};

TEST_F(UT_MyShares, Start_RegisterViewAndMenuScene_ReturnsTrue)
{
    int viewPushes = 0;
    stub.set_lamda(static_cast<PushQString>(&EventChannelManager::push),
                   [&viewPushes](EventChannelManager *, const QString &, const QString &, QString) -> QVariant {
                       __DBG_STUB_INVOKE__
                       ++viewPushes;
                       return QVariant();
                   });

    int menuPushes = 0;
    stub.set_lamda(static_cast<PushQStringQStrRRef>(&EventChannelManager::push),
                   [&menuPushes](EventChannelManager *, const QString &, const QString &, QString, QString) -> QVariant {
                       __DBG_STUB_INVOKE__
                       ++menuPushes;
                       return QVariant();
                   });

    EXPECT_TRUE(ins.start());
    EXPECT_EQ(1, viewPushes);
    EXPECT_EQ(1, menuPushes);
}

TEST_F(UT_MyShares, Initialize_DirshareNotStarted_DefersToPluginStarted)
{
    stub.set_lamda(static_cast<PluginMetaObjectPointer (*)(const QString &, const QString)>(&LifeCycle::pluginMetaObj),
                   []() -> PluginMetaObjectPointer {
                       __DBG_STUB_INVOKE__
                       return PluginMetaObjectPointer();
                   });

    bool doInitCalled = false;
    stub.set_lamda(&MyShares::doInitialize, [&doInitCalled]() {
        __DBG_STUB_INVOKE__
        doInitCalled = true;
    });

    ins.initialize();
    EXPECT_FALSE(doInitCalled);

    // unrelated plugin started: still deferred
    emit Listener::instance()->pluginStarted("org.deepin.plugin.filemanager", "dfmplugin-other");
    EXPECT_FALSE(doInitCalled);

    // dirshare started: doInitialize runs synchronously (DirectConnection)
    emit Listener::instance()->pluginStarted("org.deepin.plugin.filemanager", "dfmplugin-dirshare");
    EXPECT_TRUE(doInitCalled);
}

TEST_F(UT_MyShares, Initialize_DirshareActive_RunsDoInitialize)
{
    stub.set_lamda(static_cast<PluginMetaObjectPointer (*)(const QString &, const QString)>(&LifeCycle::pluginMetaObj),
                   []() -> PluginMetaObjectPointer {
                       __DBG_STUB_INVOKE__
                       return PluginMetaObjectPointer(new PluginMetaObject());
                   });

    stub.set_lamda(&PluginMetaObject::pluginState, []() -> PluginMetaObject::State {
        __DBG_STUB_INVOKE__
        return PluginMetaObject::kStarted;
    });

    bool doInitCalled = false;
    stub.set_lamda(&MyShares::doInitialize, [&doInitCalled]() {
        __DBG_STUB_INVOKE__
        doInitCalled = true;
    });

    ins.initialize();
    EXPECT_TRUE(doInitCalled);
}

TEST_F(UT_MyShares, Stop_DefaultImpl_NoFatalFailure)
{
    EXPECT_NO_FATAL_FAILURE(ins.stop());
}

TEST_F(UT_MyShares, OnWindowOpened_SideBarReady_AddsToSidebarImmediately)
{
    FileManagerWindow *win = new FileManagerWindow(QUrl());
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [win]() -> FileManagerWindow * {
        __DBG_STUB_INVOKE__
        return win;
    });

    stub.set_lamda(static_cast<PluginMetaObjectPointer (*)(const QString &, const QString)>(&LifeCycle::pluginMetaObj),
                   []() -> PluginMetaObjectPointer {
                       __DBG_STUB_INVOKE__
                       return PluginMetaObjectPointer();
                   });

    bool sidebarAdded = false;
    stub.set_lamda(&MyShares::addToSidebar, [&sidebarAdded]() {
        __DBG_STUB_INVOKE__
        sidebarAdded = true;
    });

    bool searchReged = false;
    stub.set_lamda(&MyShares::regMyShareToSearch, [&searchReged]() {
        __DBG_STUB_INVOKE__
        searchReged = true;
    });

    // sidebar installed: added synchronously
    stub.set_lamda(&FileManagerWindow::sideBar, []() -> AbstractFrame * {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<AbstractFrame *>(1);
    });
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(0));
    EXPECT_TRUE(sidebarAdded);
    EXPECT_FALSE(searchReged);

    // search plugin not started yet, register deferred to pluginStarted
    emit Listener::instance()->pluginStarted("org.deepin.plugin.filemanager", "dfmplugin-search");
    EXPECT_TRUE(searchReged);

    delete win;
}

TEST_F(UT_MyShares, OnWindowOpened_NoSideBar_WaitsForSideBarInstallFinished)
{
    FileManagerWindow *win = new FileManagerWindow(QUrl());
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [win]() -> FileManagerWindow * {
        __DBG_STUB_INVOKE__
        return win;
    });

    stub.set_lamda(static_cast<PluginMetaObjectPointer (*)(const QString &, const QString)>(&LifeCycle::pluginMetaObj),
                   []() -> PluginMetaObjectPointer {
                       __DBG_STUB_INVOKE__
                       return PluginMetaObjectPointer();
                   });

    int sidebarAdded = 0;
    stub.set_lamda(&MyShares::addToSidebar, [&sidebarAdded]() {
        __DBG_STUB_INVOKE__
        ++sidebarAdded;
    });

    stub.set_lamda(&MyShares::regMyShareToSearch, [] { __DBG_STUB_INVOKE__ });

    // no sidebar installed: wait for sideBarInstallFinished
    stub.set_lamda(&FileManagerWindow::sideBar, []() -> AbstractFrame * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(0));
    EXPECT_EQ(0, sidebarAdded);

    emit win->sideBarInstallFinished();
    EXPECT_EQ(1, sidebarAdded);

    delete win;
}

TEST_F(UT_MyShares, OnShareAdded_AnyPath_CallsAddToSidebar)
{
    bool sidebarAdded = false;
    stub.set_lamda(&MyShares::addToSidebar, [&sidebarAdded]() {
        __DBG_STUB_INVOKE__
        sidebarAdded = true;
    });
    EXPECT_NO_FATAL_FAILURE(ins.onShareAdded(""));
    EXPECT_TRUE(sidebarAdded);
    EXPECT_NO_FATAL_FAILURE(ins.onShareAdded("/whatever/path"));
}

TEST_F(UT_MyShares, OnShareRemoved_AllSharesGone_RemovesSidebarItem)
{
    int removePushes = 0;
    stub.set_lamda(static_cast<PushQUrl>(&EventChannelManager::push),
                   [&removePushes](EventChannelManager *, const QString &, const QString &, QUrl) -> QVariant {
                       __DBG_STUB_INVOKE__
                       ++removePushes;
                       return QVariant();
                   });

    stub.set_lamda(static_cast<PushNoArg>(&EventChannelManager::push),
                   []() -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant();   // no share left
                   });

    EXPECT_NO_FATAL_FAILURE(ins.onShareRemoved(""));
    EXPECT_EQ(1, removePushes);
}

TEST_F(UT_MyShares, OnShareRemoved_SharesRemain_KeepsSidebarItem)
{
    int removePushes = 0;
    stub.set_lamda(static_cast<PushQUrl>(&EventChannelManager::push),
                   [&removePushes](EventChannelManager *, const QString &, const QString &, QUrl) -> QVariant {
                       __DBG_STUB_INVOKE__
                       ++removePushes;
                       return QVariant();
                   });

    stub.set_lamda(static_cast<PushNoArg>(&EventChannelManager::push),
                   []() -> QVariant {
                       __DBG_STUB_INVOKE__
                       ShareInfoList lst { ShareInfo() };   // one share still exists
                       return QVariant::fromValue<ShareInfoList>(lst);
                   });

    EXPECT_NO_FATAL_FAILURE(ins.onShareRemoved("whatever/path"));
    EXPECT_EQ(0, removePushes);
}

TEST_F(UT_MyShares, AddToSideBar_NoShares_SkipsInsert)
{
    int insertPushes = 0;
    stub.set_lamda(static_cast<PushSidebarInsert>(&EventChannelManager::push),
                   [&insertPushes](EventChannelManager *, const QString &, const QString &, int, QUrl, QVariantMap &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       ++insertPushes;
                       return QVariant();
                   });

    stub.set_lamda(static_cast<PushNoArg>(&EventChannelManager::push),
                   []() -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant();   // no share infos
                   });

    EXPECT_NO_FATAL_FAILURE(ins.addToSidebar());
    EXPECT_EQ(0, insertPushes);
}

TEST_F(UT_MyShares, AddToSideBar_HasShares_InsertsToNetworkGroup)
{
    int insertPushes = 0;
    int insertIndex = -1;
    QString insertTopic;
    QVariantMap insertMap;
    stub.set_lamda(static_cast<PushSidebarInsert>(&EventChannelManager::push),
                   [&](EventChannelManager *, const QString &, const QString &topic, int index, QUrl, QVariantMap &map) -> QVariant {
                       __DBG_STUB_INVOKE__
                       ++insertPushes;
                       insertIndex = index;
                       insertTopic = topic;
                       insertMap = map;
                       return QVariant();
                   });

    stub.set_lamda(static_cast<PushNoArg>(&EventChannelManager::push),
                   []() -> QVariant {
                       __DBG_STUB_INVOKE__
                       ShareInfoList lst { ShareInfo(), ShareInfo() };
                       return QVariant::fromValue<ShareInfoList>(lst);
                   });

    EXPECT_NO_FATAL_FAILURE(ins.addToSidebar());
    EXPECT_EQ(1, insertPushes);
    EXPECT_EQ(1, insertIndex);
    EXPECT_EQ("slot_Item_Insert", insertTopic);
    EXPECT_EQ("Group_Network", insertMap.value("Property_Key_Group").toString());
    EXPECT_EQ("my_shares", insertMap.value("Property_Key_VisiableControl").toString());
    EXPECT_EQ("UserShare", insertMap.value("Property_Key_ReportName").toString());
    EXPECT_TRUE(insertMap.value("Property_Key_CallbackContextMenu").isValid());
}

TEST_F(UT_MyShares, RegMyShareToSearch_DisableSearchSet_RegistersCustomScheme)
{
    int regPushes = 0;
    QString regTopic;
    QVariantMap regMap;
    stub.set_lamda(static_cast<PushQStringMapRef>(&EventChannelManager::push),
                   [&](EventChannelManager *, const QString &space, const QString &topic, QString, QVariantMap &map) -> QVariant {
                       __DBG_STUB_INVOKE__
                       ++regPushes;
                       regTopic = topic;
                       regMap = map;
                       EXPECT_EQ(QString("dfmplugin_search"), space);
                       return QVariant();
                   });

    EXPECT_NO_FATAL_FAILURE(ins.regMyShareToSearch());
    EXPECT_EQ(1, regPushes);
    EXPECT_EQ("slot_Custom_Register", regTopic);
    EXPECT_TRUE(regMap.value("Property_Key_DisableSearch").toBool());
}

TEST_F(UT_MyShares, BeMySubScene_SceneNotRegistered_SubscribesSceneAdded)
{
    int subscribes = 0;
    stub.set_lamda(static_cast<SubscribeMyShares>(&EventDispatcherManager::subscribe),
                   [&subscribes](EventDispatcherManager *, const QString &, const QString &, MyShares *, void (MyShares::*)(const QString &)) -> bool {
                       __DBG_STUB_INVOKE__
                       ++subscribes;
                       return true;
                   });

    stub.set_lamda(static_cast<PushQString>(&EventChannelManager::push),
                   [](EventChannelManager *, const QString &, const QString &, QString) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant();   // scene not registered
                   });

    ins.eventSubscribed = false;
    ins.waitToBind.clear();
    EXPECT_NO_FATAL_FAILURE(ins.beMySubScene("hello"));
    EXPECT_EQ(1, subscribes);
    EXPECT_TRUE(ins.eventSubscribed);
    EXPECT_TRUE(ins.waitToBind.contains("hello"));

    // second unknown scene: already subscribed, no more subscription
    EXPECT_NO_FATAL_FAILURE(ins.beMySubScene("world"));
    EXPECT_EQ(1, subscribes);
    EXPECT_TRUE(ins.waitToBind.contains("world"));
}

TEST_F(UT_MyShares, BeMySubScene_SceneRegistered_BindsSubScene)
{
    int bindPushes = 0;
    QString bindParent;
    stub.set_lamda(static_cast<PushQStringQStrCRef>(&EventChannelManager::push),
                   [&bindPushes, &bindParent](EventChannelManager *, const QString &, const QString &, QString, const QString &parent) -> QVariant {
                       __DBG_STUB_INVOKE__
                       ++bindPushes;
                       bindParent = parent;
                       return QVariant::fromValue(true);
                   });

    stub.set_lamda(static_cast<PushQString>(&EventChannelManager::push),
                   [](EventChannelManager *, const QString &, const QString &, QString) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant::fromValue(true);   // scene registered
                   });

    EXPECT_NO_FATAL_FAILURE(ins.beMySubScene("hello"));
    EXPECT_EQ(1, bindPushes);
    EXPECT_EQ(MyShareMenuCreator::name(), bindParent);
}

TEST_F(UT_MyShares, BeMySubOnAdded_SceneWaiting_BindsSceneAndUnsubscribes)
{
    int bindCalls = 0;
    stub.set_lamda(&MyShares::beMySubScene, [&bindCalls](void *, const QString &) {
        __DBG_STUB_INVOKE__
        ++bindCalls;
    });

    int unsubscribes = 0;
    stub.set_lamda(static_cast<SubscribeMyShares>(&EventDispatcherManager::unsubscribe),
                   [&unsubscribes](EventDispatcherManager *, const QString &, const QString &, MyShares *, void (MyShares::*)(const QString &)) -> bool {
                       __DBG_STUB_INVOKE__
                       ++unsubscribes;
                       return true;
                   });

    ins.waitToBind.clear();
    ins.waitToBind.insert("hello");
    ins.eventSubscribed = true;
    EXPECT_NO_FATAL_FAILURE(ins.beMySubOnAdded("hello"));
    EXPECT_EQ(1, bindCalls);
    EXPECT_EQ(1, unsubscribes);
    EXPECT_TRUE(ins.waitToBind.isEmpty());
    EXPECT_FALSE(ins.eventSubscribed);
}

TEST_F(UT_MyShares, BeMySubOnAdded_MoreScenesWaiting_KeepsSubscription)
{
    int bindCalls = 0;
    stub.set_lamda(&MyShares::beMySubScene, [&bindCalls](void *, const QString &) {
        __DBG_STUB_INVOKE__
        ++bindCalls;
    });

    int unsubscribes = 0;
    stub.set_lamda(static_cast<SubscribeMyShares>(&EventDispatcherManager::unsubscribe),
                   [&unsubscribes](EventDispatcherManager *, const QString &, const QString &, MyShares *, void (MyShares::*)(const QString &)) -> bool {
                       __DBG_STUB_INVOKE__
                       ++unsubscribes;
                       return true;
                   });

    ins.waitToBind.clear();
    ins.waitToBind.insert("hello");
    ins.waitToBind.insert("world");
    EXPECT_NO_FATAL_FAILURE(ins.beMySubOnAdded("hello"));
    EXPECT_EQ(1, bindCalls);
    EXPECT_EQ(0, unsubscribes);   // "world" still waiting

    // unknown scene: nothing happens
    EXPECT_NO_FATAL_FAILURE(ins.beMySubOnAdded("unknown"));
    EXPECT_EQ(1, bindCalls);
    EXPECT_EQ(0, unsubscribes);
}

TEST_F(UT_MyShares, HookEvent_FollowsAllHookSequences_RegistersAll)
{
    typedef bool (ShareEventHelper::*HookFn3Args)(quint64, const QList<QUrl> &, const QUrl &);   // blockDelete/blockMoveToTrash/blockPaste
    typedef bool (EventSequenceManager::*Follow3Args)(const QString &, const QString &, ShareEventHelper *, HookFn3Args);
    typedef bool (ShareEventHelper::*HookFnUrls)(const QList<QUrl> &);   // hookSendOpenWindow
    typedef bool (EventSequenceManager::*FollowUrls)(const QString &, const QString &, ShareEventHelper *, HookFnUrls);
    typedef bool (ShareEventHelper::*HookFnWinUrl)(quint64, const QUrl &);   // hookSendChangeCurrentUrl
    typedef bool (EventSequenceManager::*FollowWinUrl)(const QString &, const QString &, ShareEventHelper *, HookFnWinUrl);
    typedef bool (ShareFileHelper::*HookFnOpen)(quint64, const QList<QUrl> &);   // openFileInPlugin
    typedef bool (EventSequenceManager::*FollowOpen)(const QString &, const QString &, ShareFileHelper *, HookFnOpen);

    int follow3 = 0;
    stub.set_lamda(static_cast<Follow3Args>(&EventSequenceManager::follow),
                   [&follow3](EventSequenceManager *, const QString &, const QString &, ShareEventHelper *, HookFn3Args) -> bool {
                       __DBG_STUB_INVOKE__
                       ++follow3;
                       return true;
                   });
    int followUrls = 0;
    stub.set_lamda(static_cast<FollowUrls>(&EventSequenceManager::follow),
                   [&followUrls](EventSequenceManager *, const QString &, const QString &, ShareEventHelper *, HookFnUrls) -> bool {
                       __DBG_STUB_INVOKE__
                       ++followUrls;
                       return true;
                   });
    int followWinUrl = 0;
    stub.set_lamda(static_cast<FollowWinUrl>(&EventSequenceManager::follow),
                   [&followWinUrl](EventSequenceManager *, const QString &, const QString &, ShareEventHelper *, HookFnWinUrl) -> bool {
                       __DBG_STUB_INVOKE__
                       ++followWinUrl;
                       return true;
                   });
    int followOpen = 0;
    stub.set_lamda(static_cast<FollowOpen>(&EventSequenceManager::follow),
                   [&followOpen](EventSequenceManager *, const QString &, const QString &, ShareFileHelper *, HookFnOpen) -> bool {
                       __DBG_STUB_INVOKE__
                       ++followOpen;
                       return true;
                   });

    EXPECT_NO_FATAL_FAILURE(ins.followEvents());
    EXPECT_EQ(3, follow3);
    EXPECT_EQ(1, followUrls);
    EXPECT_EQ(1, followWinUrl);
    EXPECT_EQ(1, followOpen);
}

TEST_F(UT_MyShares, BindWindows_ExistingWindows_BindsAllAndConnectsFuture)
{
    stub.set_lamda(&FileManagerWindowsManager::windowIdList, []() -> QList<quint64> {
        __DBG_STUB_INVOKE__
        return { 1, 2 };
    });

    int openedCalls = 0;
    stub.set_lamda(&MyShares::onWindowOpened, [&openedCalls]() {
        __DBG_STUB_INVOKE__
        ++openedCalls;
    });

    EXPECT_NO_FATAL_FAILURE(ins.bindWindows());
    EXPECT_EQ(2, openedCalls);

    // future windows are connected too (DirectConnection)
    emit FMWindowsIns.windowOpened(3);
    EXPECT_EQ(3, openedCalls);
}

TEST_F(UT_MyShares, DoInitialize_RegistersFactoriesAndEvents)
{
    bool followEventsCalled = false;
    stub.set_lamda(&MyShares::followEvents, [&followEventsCalled]() {
        __DBG_STUB_INVOKE__
        followEventsCalled = true;
    });
    bool bindWindowsCalled = false;
    stub.set_lamda(&MyShares::bindWindows, [&bindWindowsCalled]() {
        __DBG_STUB_INVOKE__
        bindWindowsCalled = true;
    });

    int subscribes = 0;
    stub.set_lamda(static_cast<SubscribeMyShares>(&EventDispatcherManager::subscribe),
                   [&subscribes](EventDispatcherManager *, const QString &, const QString &, MyShares *, void (MyShares::*)(const QString &)) -> bool {
                       __DBG_STUB_INVOKE__
                       ++subscribes;
                       return true;
                   });

    stub.set_lamda(static_cast<PushQString>(&EventChannelManager::push),
                   [](EventChannelManager *, const QString &, const QString &, QString) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant();   // menu scene not registered
                   });

    int sceneRegisters = 0;
    stub.set_lamda(static_cast<PushCreatorRef>(&EventChannelManager::push),
                   [&sceneRegisters](EventChannelManager *, const QString &, const QString &, QString, AbstractSceneCreator *&creator) -> QVariant {
                       __DBG_STUB_INVOKE__
                       ++sceneRegisters;
                       if (creator)
                           delete creator;
                       return QVariant::fromValue(true);
                   });

    int propPushes = 0;
    stub.set_lamda(static_cast<PushQStringMapRef>(&EventChannelManager::push),
                   [&propPushes](EventChannelManager *, const QString &, const QString &topic, QString, QVariantMap &map) -> QVariant {
                       __DBG_STUB_INVOKE__
                       ++propPushes;
                       if (topic == "slot_View_SetCustomViewProperty")
                           EXPECT_FALSE(map.value(Global::ViewCustomKeys::kSupportTreeMode).toBool());
                       return QVariant();
                   });

    EXPECT_NO_FATAL_FAILURE(ins.doInitialize());

    // scheme routed and factories registered
    EXPECT_TRUE(UrlRoute::hasScheme(ShareUtils::scheme()));
    EXPECT_TRUE(InfoFactory::create<FileInfo>(ShareUtils::rootUrl()));
    EXPECT_TRUE(DirIteratorFactory::create<AbstractDirIterator>(ShareUtils::rootUrl(), nullptr));
    EXPECT_TRUE(WatcherFactory::create<AbstractFileWatcher>(ShareUtils::rootUrl()));

    // menu scene registered, two sub scenes deferred by subscription
    EXPECT_EQ(1, sceneRegisters);
    EXPECT_EQ(3, subscribes);   // ShareAdded + ShareRemoved + SceneAdded

    // workspace + titlebar custom properties pushed
    EXPECT_EQ(2, propPushes);

    EXPECT_TRUE(followEventsCalled);
    EXPECT_TRUE(bindWindowsCalled);
}

TEST_F(UT_MyShares, ContenxtMenuHandle_UrlExists_MenuEnabledAndLogPublished)
{
    int openWindowCalls = 0;
    stub.set_lamda(&ShareEventsCaller::sendOpenWindow, [&openWindowCalls]() {
        __DBG_STUB_INVOKE__
        ++openWindowCalls;
    });
    int openTabCalls = 0;
    stub.set_lamda(&ShareEventsCaller::sendOpenTab, [&openTabCalls]() {
        __DBG_STUB_INVOKE__
        ++openTabCalls;
    });
    bool tabAddable = true;
    quint64 addableWinId = 0;
    int addableCalls = 0;
    stub.set_lamda(&ShareEventsCaller::sendCheckTabAddable,
                   [&tabAddable, &addableWinId, &addableCalls](quint64 windowId) -> bool {
                       __DBG_STUB_INVOKE__
                       ++addableCalls;
                       addableWinId = windowId;
                       return tabAddable;
                   });

    int logPublished = 0;
    QList<QUrl> logUrls;
    stub.set_lamda(static_cast<PublishMenuLog>(&EventDispatcherManager::publish),
                   [&logPublished, &logUrls](EventDispatcherManager *, const QString &, const QString &, QString, QList<QUrl> &urls) -> bool {
                       __DBG_STUB_INVOKE__
                       ++logPublished;
                       logUrls = urls;
                       return true;
                   });

    QList<bool> actEnabled;   // actions are deleted by contenxtMenuHandle, snapshot states inside exec
    stub.set_lamda(static_cast<QAction *(QMenu::*)(const QPoint &, QAction *)>(&QMenu::exec),
                   [&actEnabled](QMenu *menu, const QPoint &, QAction *) -> QAction * {
                       __DBG_STUB_INVOKE__
                       actEnabled.clear();
                       const auto acts = menu->actions();
                       for (QAction *act : acts)
                           actEnabled.append(act->isEnabled());
                       return acts.isEmpty() ? nullptr : acts.first();
                   });

    QUrl url = QUrl::fromLocalFile("/tmp");   // exists
    EXPECT_NO_FATAL_FAILURE(MyShares::contenxtMenuHandle(1, url, QPoint(10, 10)));
    EXPECT_EQ(2, actEnabled.count());   // open-in-new-window + open-in-new-tab
    EXPECT_TRUE(actEnabled.at(0));
    EXPECT_TRUE(actEnabled.at(1));   // exists && tab addable
    EXPECT_EQ(1, addableCalls);
    EXPECT_EQ(addableWinId, quint64(1));
    EXPECT_EQ(1, logPublished);   // exec returned an action
    EXPECT_EQ(url, logUrls.first());

    // tab not addable: tab action disabled, log still published on selection
    tabAddable = false;
    EXPECT_NO_FATAL_FAILURE(MyShares::contenxtMenuHandle(1, url, QPoint(10, 10)));
    EXPECT_TRUE(actEnabled.at(0));
    EXPECT_FALSE(actEnabled.at(1));
    EXPECT_EQ(2, logPublished);
}

TEST_F(UT_MyShares, ContenxtMenuHandle_UrlNotExists_ActionsDisabledAndNoLog)
{
    stub.set_lamda(&ShareEventsCaller::sendOpenWindow, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&ShareEventsCaller::sendOpenTab, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&ShareEventsCaller::sendCheckTabAddable, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    int logPublished = 0;
    stub.set_lamda(static_cast<PublishMenuLog>(&EventDispatcherManager::publish),
                   [&logPublished](EventDispatcherManager *, const QString &, const QString &, QString, QList<QUrl> &) -> bool {
                       __DBG_STUB_INVOKE__
                       ++logPublished;
                       return true;
                   });

    QList<bool> actEnabled;   // actions are deleted by contenxtMenuHandle, snapshot states inside exec
    stub.set_lamda(static_cast<QAction *(QMenu::*)(const QPoint &, QAction *)>(&QMenu::exec),
                   [&actEnabled](QMenu *menu, const QPoint &, QAction *) -> QAction * {
                       __DBG_STUB_INVOKE__
                       actEnabled.clear();
                       const auto acts = menu->actions();
                       for (QAction *act : acts)
                           actEnabled.append(act->isEnabled());
                       return nullptr;   // nothing selected
                   });

    QUrl url = QUrl::fromLocalFile("/no/such/path/in/world");
    EXPECT_NO_FATAL_FAILURE(MyShares::contenxtMenuHandle(0, url, QPoint(0, 0)));
    EXPECT_EQ(2, actEnabled.count());
    EXPECT_FALSE(actEnabled.at(0));
    EXPECT_FALSE(actEnabled.at(1));
    EXPECT_EQ(0, logPublished);   // no action triggered
}

// 触发菜单两个 action 的 lambda（sendOpenWindow / sendOpenTab），
// exec stub 内直接 activate，覆盖 QAction::addAction(text, functor) 的槽。
TEST_F(UT_MyShares, ContenxtMenuHandle_TriggerActions_InvokesCallerLambdas)
{
    int openWindowCalls = 0;
    stub.set_lamda(&ShareEventsCaller::sendOpenWindow, [&openWindowCalls]() {
        __DBG_STUB_INVOKE__
        ++openWindowCalls;
    });
    int openTabCalls = 0;
    quint64 tabWinId = 0;
    stub.set_lamda(&ShareEventsCaller::sendOpenTab, [&openTabCalls, &tabWinId](quint64 windowId, const QUrl &) {
        __DBG_STUB_INVOKE__
        ++openTabCalls;
        tabWinId = windowId;
    });
    stub.set_lamda(&ShareEventsCaller::sendCheckTabAddable, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<QAction *(QMenu::*)(const QPoint &, QAction *)>(&QMenu::exec),
                   [](QMenu *menu, const QPoint &, QAction *) -> QAction * {
                       __DBG_STUB_INVOKE__
                       for (QAction *act : menu->actions())
                           act->activate(QAction::Trigger);
                       return nullptr;
                   });

    QUrl url = QUrl::fromLocalFile("/tmp");
    EXPECT_NO_FATAL_FAILURE(MyShares::contenxtMenuHandle(7, url, QPoint(0, 0)));
    EXPECT_EQ(1, openWindowCalls);
    EXPECT_EQ(1, openTabCalls);
    EXPECT_EQ(quint64(7), tabWinId);
}
