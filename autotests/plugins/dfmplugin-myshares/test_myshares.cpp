// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
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
#include <QObject>
#include <QPair>
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

class MySharesTestSink : public QObject
{
public:
    ShareInfoList shareInfos;
    bool menuContainsResult { false };

    int registerFileViewCalls { 0 };
    int registerMenuSceneCalls { 0 };
    int removeCalls { 0 };
    int insertCalls { 0 };
    int customRegisterCalls { 0 };
    int menuContainsCalls { 0 };
    int menuBindCalls { 0 };
    int menuRegisterCalls { 0 };
    int viewPropertyCalls { 0 };
    int titlebarPropertyCalls { 0 };
    int logCalls { 0 };

    QString lastRegisterFileViewScheme;
    QString lastRegisterMenuSceneScheme;
    QString lastRegisterMenuSceneName;
    QString lastMenuScene;
    QString lastMenuParent;
    QString lastRegisterSceneName;
    QUrl lastRemovedUrl;
    int lastInsertIndex { -1 };
    QUrl lastInsertUrl;
    QVariantMap lastInsertMap;
    QVariantMap lastCustomRegisterMap;
    QVariantMap lastViewPropertyMap;
    QVariantMap lastTitlebarPropertyMap;
    QString lastLogText;
    QList<QUrl> lastLogUrls;

    bool registerFileView(const QString &scheme)
    {
        ++registerFileViewCalls;
        lastRegisterFileViewScheme = scheme;
        return true;
    }

    bool registerMenuScene(const QString &scheme, const QString &name)
    {
        ++registerMenuSceneCalls;
        lastRegisterMenuSceneScheme = scheme;
        lastRegisterMenuSceneName = name;
        return true;
    }

    ShareInfoList allShareInfos()
    {
        return shareInfos;
    }

    bool removeItem(const QUrl &url)
    {
        ++removeCalls;
        lastRemovedUrl = url;
        return true;
    }

    bool insertItem(int index, const QUrl &url, const QVariantMap &map)
    {
        ++insertCalls;
        lastInsertIndex = index;
        lastInsertUrl = url;
        lastInsertMap = map;
        return true;
    }

    bool customRegister(const QString &scheme, const QVariantMap &map)
    {
        Q_UNUSED(scheme)
        ++customRegisterCalls;
        lastCustomRegisterMap = map;
        return true;
    }

    bool menuContains(const QString &scene)
    {
        ++menuContainsCalls;
        lastMenuScene = scene;
        return menuContainsResult;
    }

    bool menuBind(const QString &scene, const QString &parent)
    {
        ++menuBindCalls;
        lastMenuScene = scene;
        lastMenuParent = parent;
        return true;
    }

    bool registerScene(const QString &name, AbstractSceneCreator *creator)
    {
        ++menuRegisterCalls;
        lastRegisterSceneName = name;
        if (creator)
            delete creator;
        return true;
    }

    bool viewProperty(const QString &scheme, const QVariantMap &map)
    {
        Q_UNUSED(scheme)
        ++viewPropertyCalls;
        lastViewPropertyMap = map;
        return true;
    }

    bool titlebarProperty(const QString &scheme, const QVariantMap &map)
    {
        Q_UNUSED(scheme)
        ++titlebarPropertyCalls;
        lastTitlebarPropertyMap = map;
        return true;
    }

    bool reportLog(const QString &text, const QList<QUrl> &urls)
    {
        ++logCalls;
        lastLogText = text;
        lastLogUrls = urls;
        return true;
    }
};

class UT_MyShares : public testing::Test
{
public:
    virtual void SetUp() override
    {
        ensureEvent(EventStratege::kSlot, "dfmplugin_workspace", "slot_RegisterFileView");
        ensureEvent(EventStratege::kSlot, "dfmplugin_workspace", "slot_RegisterMenuScene");
        ensureEvent(EventStratege::kSlot, "dfmplugin_dirshare", "slot_Share_AllShareInfos");
        ensureEvent(EventStratege::kSlot, "dfmplugin_sidebar", "slot_Item_Remove");
        ensureEvent(EventStratege::kSlot, "dfmplugin_sidebar", "slot_Item_Insert");
        ensureEvent(EventStratege::kSlot, "dfmplugin_search", "slot_Custom_Register");
        ensureEvent(EventStratege::kSlot, "dfmplugin_menu", "slot_MenuScene_Contains");
        ensureEvent(EventStratege::kSlot, "dfmplugin_menu", "slot_MenuScene_Bind");
        ensureEvent(EventStratege::kSlot, "dfmplugin_menu", "slot_MenuScene_RegisterScene");
        ensureEvent(EventStratege::kSlot, "dfmplugin_workspace", "slot_View_SetCustomViewProperty");
        ensureEvent(EventStratege::kSlot, "dfmplugin_titlebar", "slot_Custom_Register");

        ensureEvent(EventStratege::kSignal, "dfmplugin_menu", "signal_MenuScene_SceneAdded");
        ensureEvent(EventStratege::kSignal, "dfmplugin_dirshare", "signal_Share_ShareAdded");
        ensureEvent(EventStratege::kSignal, "dfmplugin_dirshare", "signal_Share_ShareRemoved");
        ensureEvent(EventStratege::kSignal, "dfmplugin_myshares", "signal_ReportLog_MenuData");

        ensureEvent(EventStratege::kHook, "dfmplugin_workspace", "hook_ShortCut_DeleteFiles");
        ensureEvent(EventStratege::kHook, "dfmplugin_workspace", "hook_ShortCut_MoveToTrash");
        ensureEvent(EventStratege::kHook, "dfmplugin_workspace", "hook_ShortCut_PasteFiles");
        ensureEvent(EventStratege::kHook, "dfmplugin_workspace", "hook_SendOpenWindow");
        ensureEvent(EventStratege::kHook, "dfmplugin_workspace", "hook_SendChangeCurrentUrl");
        ensureEvent(EventStratege::kHook, "dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin");
        ensureEvent(EventStratege::kHook, "dfmplugin_fileoperations", "hook_Operation_SetPermission");

        connectSlot("dfmplugin_workspace", "slot_RegisterFileView", &MySharesTestSink::registerFileView);
        connectSlot("dfmplugin_workspace", "slot_RegisterMenuScene", &MySharesTestSink::registerMenuScene);
        connectSlot("dfmplugin_dirshare", "slot_Share_AllShareInfos", &MySharesTestSink::allShareInfos);
        connectSlot("dfmplugin_sidebar", "slot_Item_Remove", &MySharesTestSink::removeItem);
        connectSlot("dfmplugin_sidebar", "slot_Item_Insert", &MySharesTestSink::insertItem);
        connectSlot("dfmplugin_search", "slot_Custom_Register", &MySharesTestSink::customRegister);
        connectSlot("dfmplugin_menu", "slot_MenuScene_Contains", &MySharesTestSink::menuContains);
        connectSlot("dfmplugin_menu", "slot_MenuScene_Bind", &MySharesTestSink::menuBind);
        connectSlot("dfmplugin_menu", "slot_MenuScene_RegisterScene", &MySharesTestSink::registerScene);
        connectSlot("dfmplugin_workspace", "slot_View_SetCustomViewProperty", &MySharesTestSink::viewProperty);
        connectSlot("dfmplugin_titlebar", "slot_Custom_Register", &MySharesTestSink::titlebarProperty);

        ASSERT_TRUE(dpfSignalDispatcher->subscribe("dfmplugin_myshares", "signal_ReportLog_MenuData", &sink, &MySharesTestSink::reportLog));
    }

    virtual void TearDown() override
    {
        dpfSignalDispatcher->unsubscribe("dfmplugin_myshares", "signal_ReportLog_MenuData", &sink, &MySharesTestSink::reportLog);
        for (const auto &pair : connectedSlots)
            dpfSlotChannel->disconnect(pair.first, pair.second);
        connectedSlots.clear();
        stub.clear();
    }

    static void ensureEvent(EventStratege strategy, const QString &space, const QString &topic)
    {
        if (Event::instance()->eventType(space, topic) == EventTypeScope::kInValid)
            Event::instance()->registerEventType(strategy, space, topic);
    }

    template<class Func>
    void connectSlot(const QString &space, const QString &topic, Func method)
    {
        dpfSlotChannel->connect(space, topic, &sink, method);
        connectedSlots.append(qMakePair(space, topic));
    }

    stub_ext::StubExt stub;
    MyShares ins;
    MySharesTestSink sink;
    QList<QPair<QString, QString>> connectedSlots;
};

TEST_F(UT_MyShares, Start_RegisterViewAndMenuScene_ReturnsTrue)
{
    EXPECT_TRUE(ins.start());
    EXPECT_EQ(1, sink.registerFileViewCalls);
    EXPECT_EQ(1, sink.registerMenuSceneCalls);
    EXPECT_EQ(ShareUtils::scheme(), sink.lastRegisterFileViewScheme);
    EXPECT_EQ(ShareUtils::scheme(), sink.lastRegisterMenuSceneScheme);
    EXPECT_EQ(MyShareMenuCreator::name(), sink.lastRegisterMenuSceneName);
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
    sink.shareInfos.clear();

    EXPECT_NO_FATAL_FAILURE(ins.onShareRemoved(""));
    EXPECT_EQ(1, sink.removeCalls);
    EXPECT_EQ(ShareUtils::rootUrl(), sink.lastRemovedUrl);
}

TEST_F(UT_MyShares, OnShareRemoved_SharesRemain_KeepsSidebarItem)
{
    sink.shareInfos = ShareInfoList { ShareInfo() };

    EXPECT_NO_FATAL_FAILURE(ins.onShareRemoved("whatever/path"));
    EXPECT_EQ(0, sink.removeCalls);
}

TEST_F(UT_MyShares, AddToSideBar_NoShares_SkipsInsert)
{
    sink.shareInfos.clear();

    EXPECT_NO_FATAL_FAILURE(ins.addToSidebar());
    EXPECT_EQ(0, sink.insertCalls);
}

TEST_F(UT_MyShares, AddToSideBar_HasShares_InsertsToNetworkGroup)
{
    sink.shareInfos = ShareInfoList { ShareInfo(), ShareInfo() };

    EXPECT_NO_FATAL_FAILURE(ins.addToSidebar());
    EXPECT_EQ(1, sink.insertCalls);
    EXPECT_EQ(1, sink.lastInsertIndex);
    EXPECT_EQ(ShareUtils::rootUrl(), sink.lastInsertUrl);
    EXPECT_EQ("Group_Network", sink.lastInsertMap.value("Property_Key_Group").toString());
    EXPECT_EQ("my_shares", sink.lastInsertMap.value("Property_Key_VisiableControl").toString());
    EXPECT_EQ("UserShare", sink.lastInsertMap.value("Property_Key_ReportName").toString());
    EXPECT_TRUE(sink.lastInsertMap.value("Property_Key_CallbackContextMenu").isValid());
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

    sink.menuContainsResult = false;

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
    sink.menuContainsResult = true;

    EXPECT_NO_FATAL_FAILURE(ins.beMySubScene("hello"));
    EXPECT_EQ(1, sink.menuBindCalls);
    EXPECT_EQ(MyShareMenuCreator::name(), sink.lastMenuParent);
    EXPECT_TRUE(ins.waitToBind.isEmpty());
}

TEST_F(UT_MyShares, BeMySubOnAdded_SceneWaiting_BindsSceneAndUnsubscribes)
{
    int bindCalls = 0;
    stub.set_lamda(&MyShares::beMySubScene, [&bindCalls](void *, const QString &) {
        __DBG_STUB_INVOKE__
        ++bindCalls;
    });

    ASSERT_TRUE(dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", &ins, &MyShares::beMySubOnAdded));

    ins.waitToBind.clear();
    ins.waitToBind.insert("hello");
    ins.eventSubscribed = true;
    EXPECT_NO_FATAL_FAILURE(ins.beMySubOnAdded("hello"));
    EXPECT_EQ(1, bindCalls);
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

    ASSERT_TRUE(dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", &ins, &MyShares::beMySubOnAdded));

    ins.waitToBind.clear();
    ins.waitToBind.insert("hello");
    ins.waitToBind.insert("world");
    ins.eventSubscribed = true;
    EXPECT_NO_FATAL_FAILURE(ins.beMySubOnAdded("hello"));
    EXPECT_EQ(1, bindCalls);
    EXPECT_TRUE(ins.eventSubscribed);   // "world" still waiting

    // unknown scene: nothing happens
    EXPECT_NO_FATAL_FAILURE(ins.beMySubOnAdded("unknown"));
    EXPECT_EQ(1, bindCalls);
    EXPECT_TRUE(ins.eventSubscribed);
}

TEST_F(UT_MyShares, HookEvent_FollowsAllHookSequences_RegistersAll)
{
    QUrl shareUrl;
    shareUrl.setScheme(ShareUtils::scheme());
    shareUrl.setPath("/hello");

    EXPECT_NO_FATAL_FAILURE(ins.followEvents());

    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_workspace", "hook_ShortCut_DeleteFiles",
                                     quint64(0), QList<QUrl> { shareUrl }, QUrl()));
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_workspace", "hook_ShortCut_MoveToTrash",
                                     quint64(0), QList<QUrl> { shareUrl }, QUrl()));
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_workspace", "hook_ShortCut_PasteFiles",
                                     quint64(0), QList<QUrl> {}, shareUrl));
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_workspace", "hook_SendOpenWindow",
                                     QList<QUrl> { shareUrl }));
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_workspace", "hook_SendChangeCurrentUrl",
                                     quint64(0), shareUrl));
    EXPECT_TRUE(dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin",
                                     quint64(0), QList<QUrl> { shareUrl }));
    EXPECT_TRUE(Event::instance()->eventType("dfmplugin_fileoperations", "hook_Operation_SetPermission") > 0);
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

    sink.menuContainsResult = false;

    EXPECT_NO_FATAL_FAILURE(ins.doInitialize());

    // scheme routed and factories registered
    EXPECT_TRUE(UrlRoute::hasScheme(ShareUtils::scheme()));
    EXPECT_TRUE(InfoFactory::create<FileInfo>(ShareUtils::rootUrl()));
    EXPECT_TRUE(DirIteratorFactory::create<AbstractDirIterator>(ShareUtils::rootUrl(), nullptr));
    EXPECT_TRUE(WatcherFactory::create<AbstractFileWatcher>(ShareUtils::rootUrl()));

    // menu scene registered, two sub scenes deferred by subscription
    EXPECT_EQ(1, sink.menuRegisterCalls);
    EXPECT_EQ(2, sink.menuContainsCalls);
    EXPECT_EQ(3, subscribes);   // ShareAdded + ShareRemoved + SceneAdded

    // workspace + titlebar custom properties pushed
    EXPECT_EQ(1, sink.viewPropertyCalls);
    EXPECT_EQ(1, sink.titlebarPropertyCalls);
    EXPECT_FALSE(sink.lastViewPropertyMap.value(Global::ViewCustomKeys::kSupportTreeMode).toBool());

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
    EXPECT_EQ(1, sink.logCalls);   // exec returned an action
    EXPECT_EQ(url, sink.lastLogUrls.first());

    // tab not addable: tab action disabled, log still published on selection
    tabAddable = false;
    EXPECT_NO_FATAL_FAILURE(MyShares::contenxtMenuHandle(1, url, QPoint(10, 10)));
    EXPECT_TRUE(actEnabled.at(0));
    EXPECT_FALSE(actEnabled.at(1));
    EXPECT_EQ(2, sink.logCalls);
}

TEST_F(UT_MyShares, ContenxtMenuHandle_UrlNotExists_ActionsDisabledAndNoLog)
{
    stub.set_lamda(&ShareEventsCaller::sendOpenWindow, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&ShareEventsCaller::sendOpenTab, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&ShareEventsCaller::sendCheckTabAddable, []() -> bool {
        __DBG_STUB_INVOKE__
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
    EXPECT_EQ(0, sink.logCalls);   // no action triggered
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
