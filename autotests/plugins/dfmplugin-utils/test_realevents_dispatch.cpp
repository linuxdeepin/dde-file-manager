// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Real event-dispatch tests for dfmplugin-utils: wire the production
// subscriptions (initEventConnect / bindEvents / initializeConnections /
// plugin initialize+start) after registering every slot/hook/signal topic,
// then publish / push / run the real events with exactly-typed QVariant
// arguments so every EventHelper<M>::invoke, EventHelper<M>::EventHelper and
// memberFunctionVoidCast<M> template instantiation owned by dfmplugin_utils
// in include/dfm-framework/event/eventhelper.h actually executes. Heavy
// dependencies (QProcess, pkexec dialog, bluetooth manager, report-log
// manager, append-compress helper, extension plugin loader) are stubbed and
// their arguments captured for exact-value assertions.

#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QMimeData>
#include <QPoint>
#include <QUrl>
#include <QIcon>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QProcess>
#include <QAccessible>

#include "stubext.h"

#include "plugins/common/dfmplugin-utils/global/globaleventreceiver.h"
#include "plugins/common/dfmplugin-utils/vaultassist/vaulthelperreceiver.h"
#include "plugins/common/dfmplugin-utils/vaultassist/vaultassitcontrol.h"
#include "plugins/common/dfmplugin-utils/testing/events/testingeventrecevier.h"
#include "plugins/common/dfmplugin-utils/openwith/openwitheventreceiver.h"
#include "plugins/common/dfmplugin-utils/openwith/openwithdialog.h"
#include "plugins/common/dfmplugin-utils/reportlog/reportlogeventreceiver.h"
#include "plugins/common/dfmplugin-utils/reportlog/reportlogmanager.h"
#include "plugins/common/dfmplugin-utils/appendcompress/appendcompresseventreceiver.h"
#include "plugins/common/dfmplugin-utils/appendcompress/appendcompresshelper.h"
#include "plugins/common/dfmplugin-utils/bluetooth/virtualbluetoothplugin.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothmanager.h"
#include "plugins/common/dfmplugin-utils/bluetooth/private/bluetoothtransdialog.h"
#include "plugins/common/dfmplugin-utils/shred/vitrualshredplugin.h"
#include "plugins/common/dfmplugin-utils/shred/shredutils.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/virtualextensionimplplugin.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/emblemimpl/extensionemblemmanager.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/pluginsload/extensionpluginmanager.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-framework/dpf.h>
#include <dfm-framework/event/event.h>

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

Q_DECLARE_METATYPE(QList<QIcon> *)
Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(const QMimeData *)

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_utils;

namespace {

// register every topic the production code subscribes to; registration is
// idempotent (repeat calls only warn) and guarded by a process-wide once flag.
std::once_flag g_topicOnce;
void ensureTopicsRegistered()
{
    std::call_once(g_topicOnce, [] {
        auto reg = [](EventStratege stratege, const char *space, const char *topic) {
            dpf::Event::instance()->registerEventType(stratege, space, topic);
        };
        // signal topics
        reg(EventStratege::kSignal, "dfmplugin_menu", "signal_MenuScene_SceneAdded");
        reg(EventStratege::kSignal, "dfmplugin_workspace", "signal_ReportLog_Commit");
        reg(EventStratege::kSignal, "dfmplugin_workspace", "signal_ReportLog_MenuData");
        reg(EventStratege::kSignal, "ddplugin_background", "signal_ReportLog_BackgroundPaint");
        // hook topics
        reg(EventStratege::kHook, "dfmplugin_fileoperations", "hook_Operation_MoveToTrash");
        reg(EventStratege::kHook, "dfmplugin_emblem", "hook_ExtendEmblems_Fetch");
        reg(EventStratege::kHook, "dfmplugin_workspace", "hook_DragDrop_FileDragMove");
        reg(EventStratege::kHook, "dfmplugin_workspace", "hook_DragDrop_FileDrop");
        reg(EventStratege::kHook, "dfmplugin_workspace", "hook_DragDrop_IsDrop");
        reg(EventStratege::kHook, "ddplugin_canvas", "hook_CanvasView_DragMove");
        reg(EventStratege::kHook, "ddplugin_canvas", "hook_CanvasView_DropData");
        reg(EventStratege::kHook, "ddplugin_organizer", "hook_CollectionView_DragMove");
        reg(EventStratege::kHook, "ddplugin_organizer", "hook_CollectionView_DropData");
        // slot topics
        reg(EventStratege::kSlot, "dfmplugin_utils", "slot_OpenWith_ShowDialog");
        reg(EventStratege::kSlot, "dfmplugin_utils", "slot_Bluetooth_IsAvailable");
        reg(EventStratege::kSlot, "dfmplugin_utils", "slot_Bluetooth_SendFiles");
        reg(EventStratege::kSlot, "dfmplugin_utils", "slot_Accessible_SetAccessibleName");
        reg(EventStratege::kSlot, "dfmplugin_utils", "slot_Accessible_SetObjectName");
        // menu protocol slots used by the scene-bind plugins (inline helpers
        // cannot be stubbed, so they are observed through a real receiver)
        reg(EventStratege::kSlot, "dfmplugin_menu", "slot_MenuScene_Contains");
        reg(EventStratege::kSlot, "dfmplugin_menu", "slot_MenuScene_Bind");
        reg(EventStratege::kSlot, "dfmplugin_menu", "slot_MenuScene_RegisterScene");
    });
}

// Real receiver for the dfmplugin_menu scene protocol. The inline helpers in
// menu_eventinterface_helper.h push these slots; returning false from
// onSceneContains makes the plugins take the "scene not present yet" branch
// (subscribe + wait), and onSceneBind records what actually got bound.
class MenuSceneSpy : public QObject
{
    Q_OBJECT
public:
    int containsCalls = 0;
    int bindCalls = 0;
    QString boundName, boundParent;

    bool containsResult = false;

public slots:
    bool onSceneContains(const QString &name)
    {
        Q_UNUSED(name)
        containsCalls++;
        return containsResult;
    }
    bool onSceneBind(const QString &name, const QString &parent)
    {
        bindCalls++;
        boundName = name;
        boundParent = parent;
        return true;
    }
};

// ---------- captured arguments / call flags (reset in SetUp) ----------
bool g_detachCalled = false;
bool g_isVaultCalled = false;
bool g_transCalled = false;
bool g_mouseStyleCalled = false;
bool g_compressCalled = false;
bool g_compressIsDrop = false;
bool g_commitCalled = false;
bool g_menuDataCalled = false;
bool g_startupCalled = false;
bool g_msgDialogShown = false;
int g_execCount = 0;
int g_msgDialogCount = 0;

QString g_detachProgram;
QStringList g_detachArgs;
QList<QUrl> g_transInput;
QList<QUrl> g_mouseFromUrls;
QUrl g_mouseToUrl;
QList<QUrl> g_compressFromUrls;
QUrl g_compressToUrl;
QString g_commitType;
QVariantMap g_commitArgs;
QString g_menuDataName;
QList<QUrl> g_menuDataUrls;
QString g_startupKey;
QVariant g_startupData;
QString g_msgDialogTitle;

void resetCaptures()
{
    g_detachCalled = g_isVaultCalled = g_transCalled = false;
    g_mouseStyleCalled = g_compressCalled = g_compressIsDrop = false;
    g_commitCalled = g_menuDataCalled = g_startupCalled = false;
    g_msgDialogShown = false;
    g_execCount = 0;
    g_msgDialogCount = 0;

    g_detachProgram = QString();
    g_detachArgs.clear();
    g_transInput.clear();
    g_mouseFromUrls.clear();
    g_mouseToUrl = QUrl();
    g_compressFromUrls.clear();
    g_compressToUrl = QUrl();
    g_commitType = QString();
    g_commitArgs.clear();
    g_menuDataName = QString();
    g_menuDataUrls.clear();
    g_startupKey = QString();
    g_startupData = QVariant();
    g_msgDialogTitle = QString();
}

}   // namespace

class UtilsEventDispatchTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        resetCaptures();
        ensureTopicsRegistered();

        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
        // detach every slot channel this fixture connected so later test
        // files never push into a destroyed stack receiver
        for (const char *topic : { "slot_MenuScene_Contains", "slot_MenuScene_Bind",
                                   "slot_MenuScene_RegisterScene", "slot_OpenWith_ShowDialog",
                                   "slot_Bluetooth_IsAvailable", "slot_Bluetooth_SendFiles",
                                   "slot_Accessible_SetAccessibleName", "slot_Accessible_SetObjectName" }) {
            dpfSlotChannel->disconnect("dfmplugin_menu", topic);
            dpfSlotChannel->disconnect("dfmplugin_utils", topic);
        }
    }

    QUrl createTestFile(const QString &name)
    {
        QString path = tempDir->path() + "/" + name;
        QFile f(path);
        f.open(QIODevice::WriteOnly);
        f.write("dispatch");
        f.close();
        return QUrl::fromLocalFile(path);
    }

    void installShredStartupStubs()
    {
        stub.set_lamda(ADDR(ShredUtils, initDconfig), [](ShredUtils *) {});
        stub.set_lamda(ADDR(SettingJsonGenerator, addGroup),
                       [](SettingJsonGenerator *, const QString &, const QString &) { return true; });
        stub.set_lamda(ADDR(SettingJsonGenerator, addConfig),
                       [](SettingJsonGenerator *, const QString &, const QVariantMap &) { return true; });
        stub.set_lamda(ADDR(DialogManager, registerSettingWidget),
                       [](DialogManager *, const QString &, std::function<QWidget *(QObject *)>) {});
        stub.set_lamda(&LifeCycle::isAllPluginsStarted, []() -> bool { return true; });
    }

    // wire the menu protocol to a fresh spy: scenes always "not present" so
    // plugins subscribe and wait; binds get recorded for assertions
    MenuSceneSpy &connectMenuSpy(MenuSceneSpy &spy, bool containsResult = false)
    {
        spy.containsResult = containsResult;
        dpfSlotChannel->connect("dfmplugin_menu", "slot_MenuScene_Contains", &spy, &MenuSceneSpy::onSceneContains);
        dpfSlotChannel->connect("dfmplugin_menu", "slot_MenuScene_Bind", &spy, &MenuSceneSpy::onSceneBind);
        return spy;
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
};

// ========== GlobalEventReceiver: kOpenAsAdmin signal ==========

TEST_F(UtilsEventDispatchTest, OpenAsAdminEvent_PublishLocalFile_PkexecLaunchedWithExactPath)
{
    stub.set_lamda(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(&QProcess::startDetached),
                   [](const QString &program, const QStringList &args, const QString &, qint64 *) -> bool {
                       g_detachCalled = true;
                       g_detachProgram = program;
                       g_detachArgs = args;
                       return true;
                   });

    QUrl target = createTestFile("admin_target.txt");

    GlobalEventReceiver receiver;
    receiver.initEventConnect();

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kOpenAsAdmin, target);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_detachCalled);
    EXPECT_EQ(g_detachProgram, QString("dde-file-manager-pkexec"));
    EXPECT_EQ(g_detachArgs, QStringList { target.toLocalFile() });
}

TEST_F(UtilsEventDispatchTest, OpenAsAdminEvent_PublishEmptyUrl_NoProcessLaunched)
{
    stub.set_lamda(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(&QProcess::startDetached),
                   [](const QString &, const QStringList &, const QString &, qint64 *) -> bool {
                       g_detachCalled = true;
                       return true;
                   });

    GlobalEventReceiver receiver;
    receiver.initEventConnect();

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kOpenAsAdmin, QUrl());

    EXPECT_TRUE(published);
    EXPECT_FALSE(g_detachCalled);
}

// ========== VaultHelperReceiver: hook_Operation_MoveToTrash ==========

TEST_F(UtilsEventDispatchTest, MoveToTrashHook_RunVaultSources_ReturnsTrueAndTranslatesUrls)
{
    QList<QUrl> sources { QUrl::fromLocalFile("/vault/fileA") };
    stub.set_lamda(ADDR(VaultAssitControl, isVaultFile),
                   [&sources](VaultAssitControl *, const QUrl &url) -> bool {
                       g_isVaultCalled = true;
                       return url == sources.first();
                   });
    stub.set_lamda(ADDR(VaultAssitControl, transUrlsToLocal),
                   [&sources](VaultAssitControl *, const QList<QUrl> &urls) -> QList<QUrl> {
                       g_transCalled = true;
                       g_transInput = urls;
                       return QList<QUrl> { QUrl::fromLocalFile("/root/.vault/fileA") };
                   });

    VaultHelperReceiver receiver;
    receiver.initEventConnect();

    bool handled = dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_MoveToTrash",
                                        quint64(42), sources,
                                        AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_TRUE(handled);
    ASSERT_TRUE(g_isVaultCalled);
    ASSERT_TRUE(g_transCalled);
    EXPECT_EQ(g_transInput, sources);
}

TEST_F(UtilsEventDispatchTest, MoveToTrashHook_RunForeignSources_ReturnsFalse)
{
    stub.set_lamda(ADDR(VaultAssitControl, isVaultFile),
                   [](VaultAssitControl *, const QUrl &) -> bool {
                       g_isVaultCalled = true;
                       return false;
                   });

    VaultHelperReceiver receiver;
    receiver.initEventConnect();

    QList<QUrl> sources { QUrl::fromLocalFile("/home/uos/plain.txt") };
    bool handled = dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_MoveToTrash",
                                        quint64(1), sources,
                                        AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(handled);
    ASSERT_TRUE(g_isVaultCalled);
    EXPECT_FALSE(g_transCalled);
}

TEST_F(UtilsEventDispatchTest, MoveToTrashHook_RunEmptySources_ReturnsFalse)
{
    stub.set_lamda(ADDR(VaultAssitControl, isVaultFile),
                   [](VaultAssitControl *, const QUrl &) -> bool { return true; });

    VaultHelperReceiver receiver;
    receiver.initEventConnect();

    bool handled = dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_MoveToTrash",
                                        quint64(1), QList<QUrl> {},
                                        AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(handled);
    EXPECT_FALSE(g_isVaultCalled);
}

// ========== TestingEventRecevier: accessible slots ==========

TEST_F(UtilsEventDispatchTest, AccessibleSlot_PushWidgetAndName_AccessibleNameSetExact)
{
    // keep the global QAccessible factory untouched, only the slot wiring is real
    stub.set_lamda(&QAccessible::installFactory, [](QAccessible::InterfaceFactory) {});
    stub.set_lamda(&QAccessible::setActive, [](bool) {});

    TestingEventRecevier::instance()->initializeConnections();

    QWidget widget;
    QVariant ret = dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                                        &widget, QString("btn-open-42"));

    EXPECT_TRUE(ret.isNull());
    EXPECT_EQ(widget.accessibleName(), QString("btn-open-42"));
    EXPECT_NE(widget.accessibleName(), QString());
}

TEST_F(UtilsEventDispatchTest, AccessibleSlot_PushObjectName_HandlerIsNoOp)
{
    stub.set_lamda(&QAccessible::installFactory, [](QAccessible::InterfaceFactory) {});
    stub.set_lamda(&QAccessible::setActive, [](bool) {});

    TestingEventRecevier::instance()->initializeConnections();

    QWidget widget;
    QVariant ret = dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetObjectName",
                                        &widget, QString("obj-name"));

    EXPECT_TRUE(ret.isNull());
    EXPECT_TRUE(widget.objectName().isEmpty());
}

// ========== OpenWithEventReceiver: slot_OpenWith_ShowDialog ==========

TEST_F(UtilsEventDispatchTest, OpenWithSlot_PushZeroWinId_DialogExecuted)
{
    stub.set_lamda(VADDR(OpenWithDialog, exec), []() -> int {
        g_execCount++;
        return 0;
    });

    OpenWithEventReceiver receiver;
    receiver.initEventConnect();

    QList<QUrl> urls { QUrl::fromLocalFile("/tmp/openwith_target.txt") };
    QVariant ret = dpfSlotChannel->push("dfmplugin_utils", "slot_OpenWith_ShowDialog",
                                        quint64(0), urls);

    EXPECT_TRUE(ret.isNull());
    EXPECT_EQ(g_execCount, 1);
}

TEST_F(UtilsEventDispatchTest, OpenWithSlot_PushUnknownWinId_DialogExecutedWithFallbackParent)
{
    stub.set_lamda(VADDR(OpenWithDialog, exec), []() -> int {
        g_execCount++;
        return 0;
    });

    OpenWithEventReceiver receiver;
    receiver.initEventConnect();

    QList<QUrl> urls { QUrl::fromLocalFile("/tmp/openwith_fallback.txt") };
    QVariant ret = dpfSlotChannel->push("dfmplugin_utils", "slot_OpenWith_ShowDialog",
                                        quint64(987654321), urls);

    EXPECT_TRUE(ret.isNull());
    EXPECT_EQ(g_execCount, 1);
}

// ========== VirtualBluetoothPlugin: bluetooth slots ==========

TEST_F(UtilsEventDispatchTest, BluetoothAvailableSlot_PushWithAdapter_ReturnsTrue)
{
    stub.set_lamda(ADDR(SysInfoUtils, isOpenAsAdmin), []() -> bool { return false; });
    stub.set_lamda(ADDR(BluetoothManager, bluetoothSendEnable),
                   [](BluetoothManager *) -> bool { return true; });
    stub.set_lamda(ADDR(BluetoothManager, hasAdapter),
                   [](BluetoothManager *) -> bool { return true; });

    VirtualBluetoothPlugin plugin;
    plugin.initialize();

    QVariant ret = dpfSlotChannel->push("dfmplugin_utils", "slot_Bluetooth_IsAvailable");

    EXPECT_TRUE(ret.isValid());
    EXPECT_TRUE(ret.toBool());
    EXPECT_EQ(ret.typeId(), QMetaType::Type::Bool);
}

TEST_F(UtilsEventDispatchTest, BluetoothAvailableSlot_PushWithoutAdapter_ReturnsFalse)
{
    stub.set_lamda(ADDR(SysInfoUtils, isOpenAsAdmin), []() -> bool { return false; });
    stub.set_lamda(ADDR(BluetoothManager, bluetoothSendEnable),
                   [](BluetoothManager *) -> bool { return false; });
    stub.set_lamda(ADDR(BluetoothManager, hasAdapter),
                   [](BluetoothManager *) -> bool { return true; });

    VirtualBluetoothPlugin plugin;
    plugin.initialize();

    QVariant ret = dpfSlotChannel->push("dfmplugin_utils", "slot_Bluetooth_IsAvailable");

    EXPECT_TRUE(ret.isValid());
    EXPECT_FALSE(ret.toBool());
}

TEST_F(UtilsEventDispatchTest, BluetoothSendSlot_PushEmptyPaths_ReturnsWithoutDialog)
{
    stub.set_lamda(ADDR(SysInfoUtils, isOpenAsAdmin), []() -> bool { return false; });
    stub.set_lamda(ADDR(BluetoothTransDialog, isBluetoothIdle), []() -> bool { return true; });
    stub.set_lamda(static_cast<int (DialogManager::*)(const QString &, const QString &, QString)>(&DialogManager::showMessageDialog),
                   [](DialogManager *, const QString &, const QString &, const QString &) -> int {
                       g_msgDialogShown = true;
                       return 0;
                   });

    VirtualBluetoothPlugin plugin;
    plugin.initialize();

    QVariant ret = dpfSlotChannel->push("dfmplugin_utils", "slot_Bluetooth_SendFiles",
                                        QStringList {}, QString("device-1"));

    EXPECT_TRUE(ret.isNull());
    EXPECT_FALSE(g_msgDialogShown);
}

TEST_F(UtilsEventDispatchTest, BluetoothSendSlot_PushWhileBusy_ShowsWarningDialog)
{
    stub.set_lamda(ADDR(SysInfoUtils, isOpenAsAdmin), []() -> bool { return false; });
    stub.set_lamda(ADDR(BluetoothTransDialog, isBluetoothIdle), []() -> bool { return false; });
    stub.set_lamda(static_cast<int (DialogManager::*)(const QString &, const QString &, QString)>(&DialogManager::showMessageDialog),
                   [](DialogManager *, const QString &title, const QString &, const QString &) -> int {
                       g_msgDialogShown = true;
                       g_msgDialogCount++;
                       g_msgDialogTitle = title;
                       return 0;
                   });

    VirtualBluetoothPlugin plugin;
    plugin.initialize();

    QVariant ret = dpfSlotChannel->push("dfmplugin_utils", "slot_Bluetooth_SendFiles",
                                        QStringList { "/tmp/send_me.txt" }, QString("device-2"));

    EXPECT_TRUE(ret.isNull());
    ASSERT_TRUE(g_msgDialogShown);
    EXPECT_EQ(g_msgDialogCount, 1);
    EXPECT_FALSE(g_msgDialogTitle.isEmpty());
}

// ========== ReportLogEventReceiver: report-log signals ==========

TEST_F(UtilsEventDispatchTest, ReportLogCommit_PublishWorkspaceSignal_CommitForwardedWithExactArgs)
{
    stub.set_lamda(ADDR(ReportLogManager, commit),
                   [](ReportLogManager *, const QString &type, const QVariantMap &args) {
                       g_commitCalled = true;
                       g_commitType = type;
                       g_commitArgs = args;
                   });

    ReportLogEventReceiver receiver;
    receiver.bindEvents();

    QVariantMap args;
    args.insert("op", QVariant("open"));
    args.insert("count", QVariant(3));
    bool published = dpfSignalDispatcher->publish("dfmplugin_workspace", "signal_ReportLog_Commit",
                                                  QString("FileOpen"), args);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_commitCalled);
    EXPECT_EQ(g_commitType, QString("FileOpen"));
    EXPECT_EQ(g_commitArgs.value("op").toString(), QString("open"));
    EXPECT_EQ(g_commitArgs.value("count").toInt(), 3);
}

TEST_F(UtilsEventDispatchTest, ReportLogMenuData_PublishWorkspaceSignal_MenuDataForwarded)
{
    stub.set_lamda(ADDR(ReportLogManager, reportMenuData),
                   [](ReportLogManager *, const QString &name, const QList<QUrl> &urlList) {
                       g_menuDataCalled = true;
                       g_menuDataName = name;
                       g_menuDataUrls = urlList;
                   });

    ReportLogEventReceiver receiver;
    receiver.bindEvents();

    QList<QUrl> urls { createTestFile("menu_report.txt") };
    bool published = dpfSignalDispatcher->publish("dfmplugin_workspace", "signal_ReportLog_MenuData",
                                                  QString("empty-menu"), urls);

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_menuDataCalled);
    EXPECT_EQ(g_menuDataName, QString("empty-menu"));
    EXPECT_EQ(g_menuDataUrls, urls);
    EXPECT_EQ(g_menuDataUrls.size(), 1);
}

TEST_F(UtilsEventDispatchTest, ReportLogStartup_PublishBackgroundSignal_StartupForwarded)
{
    stub.set_lamda(ADDR(ReportLogManager, reportDesktopStartUp),
                   [](ReportLogManager *, const QString &key, const QVariant &data) {
                       g_startupCalled = true;
                       g_startupKey = key;
                       g_startupData = data;
                   });

    ReportLogEventReceiver receiver;
    receiver.bindEvents();

    bool published = dpfSignalDispatcher->publish("ddplugin_background", "signal_ReportLog_BackgroundPaint",
                                                  QString("loadfinish"), QVariant(1500));

    EXPECT_TRUE(published);
    ASSERT_TRUE(g_startupCalled);
    EXPECT_EQ(g_startupKey, QString("loadfinish"));
    EXPECT_EQ(g_startupData.toInt(), 1500);
}

// ========== VirtualShredPlugin: signal_MenuScene_SceneAdded ==========

TEST_F(UtilsEventDispatchTest, ShredSceneAdded_PublishBoundScene_BindsMenuAndUnsubscribes)
{
    installShredStartupStubs();

    MenuSceneSpy spy;
    connectMenuSpy(spy);

    VirtualShredPlugin plugin;
    ASSERT_TRUE(plugin.start());
    EXPECT_GE(spy.containsCalls, 1);   // bindScene consulted the scene registry

    bool published = dpfSignalDispatcher->publish("dfmplugin_menu", "signal_MenuScene_SceneAdded",
                                                  QString("FileOperatorMenu"));

    EXPECT_TRUE(published);
    ASSERT_EQ(spy.bindCalls, 1);
    EXPECT_EQ(spy.boundName, QString("ShredMenu"));
    EXPECT_EQ(spy.boundParent, QString("FileOperatorMenu"));
}

TEST_F(UtilsEventDispatchTest, ShredSceneAdded_PublishForeignScene_NothingBoundThenCleansUp)
{
    installShredStartupStubs();

    MenuSceneSpy spy;
    connectMenuSpy(spy);

    VirtualShredPlugin plugin;
    ASSERT_TRUE(plugin.start());

    bool foreign = dpfSignalDispatcher->publish("dfmplugin_menu", "signal_MenuScene_SceneAdded",
                                                QString("TotallyUnknownScene"));
    EXPECT_TRUE(foreign);
    EXPECT_EQ(spy.bindCalls, 0);   // unknown scene must not trigger a bind

    // publish the awaited scene so the plugin unsubscribes and leaves no
    // dangling handler behind after this stack object dies
    bool cleanup = dpfSignalDispatcher->publish("dfmplugin_menu", "signal_MenuScene_SceneAdded",
                                                QString("FileOperatorMenu"));
    EXPECT_TRUE(cleanup);
    ASSERT_EQ(spy.bindCalls, 1);
    EXPECT_EQ(spy.boundParent, QString("FileOperatorMenu"));
}

// ========== VirtualExtensionImplPlugin + ExtensionEmblemManager ==========

TEST_F(UtilsEventDispatchTest, ExtensionSceneAdded_PublishWaitedScene_BindsExtensionMenu)
{
    installShredStartupStubs();   // shares lifecycle/setting stubs

    // never load real dfmext plugins from the host system
    stub.set_lamda(ADDR(ExtensionPluginManager, onLoadingPlugins), [](ExtensionPluginManager *) {});

    MenuSceneSpy spy;
    // first Contains (from start) says "not present"; the second Contains
    // (re-bind inside bindSceneOnAdded) says "present" so the plugin stops
    // resubscribing and leaves no dangling handler behind
    connectMenuSpy(spy);

    VirtualExtensionImplPlugin plugin;
    plugin.initialize();
    ASSERT_TRUE(plugin.start());

    int containsBefore = spy.containsCalls;
    spy.containsResult = true;   // next Contains flips to "scene present"

    bool published = dpfSignalDispatcher->publish("dfmplugin_menu", "signal_MenuScene_SceneAdded",
                                                  QString("ExtendMenu"));

    EXPECT_TRUE(published);
    EXPECT_GT(spy.containsCalls, containsBefore);   // re-bind consulted the registry again
    ASSERT_EQ(spy.bindCalls, 1);
    EXPECT_EQ(spy.boundName, QString("ExtensionLibMenu"));
    EXPECT_EQ(spy.boundParent, QString("ExtendMenu"));

    // followEvents() wired the emblem hook during initialize(): running it
    // must reach ExtensionEmblemManager::onFetchCustomEmblems, which returns
    // false because extension plugins are never initialized in tests.
    QList<QIcon> emblems;
    bool fetched = dpfHookSequence->run("dfmplugin_emblem", "hook_ExtendEmblems_Fetch",
                                        QUrl::fromLocalFile("/tmp/emblem_target.txt"), &emblems);
    EXPECT_FALSE(fetched);
    EXPECT_TRUE(emblems.isEmpty());
}

TEST_F(UtilsEventDispatchTest, EmblemFetchHook_RunValidUrlWithoutPlugins_ReturnsFalseKeepsEmblems)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, onLoadingPlugins), [](ExtensionPluginManager *) {});

    VirtualExtensionImplPlugin plugin;
    plugin.initialize();   // wires hook_ExtendEmblems_Fetch via followEvents()

    QList<QIcon> emblems { QIcon() };
    int countBefore = emblems.size();
    bool fetched = dpfHookSequence->run("dfmplugin_emblem", "hook_ExtendEmblems_Fetch",
                                        QUrl::fromLocalFile("/tmp/emblem_plain.txt"), &emblems);

    EXPECT_FALSE(fetched);
    EXPECT_EQ(emblems.size(), countBefore);
}

TEST_F(UtilsEventDispatchTest, ChangeCurrentUrlEvent_PublishAnyUrl_ClearsCacheAndRequestsClear)
{
    ExtensionEmblemManager::instance().initialize();

    QSignalSpy spy(&ExtensionEmblemManager::instance(), &ExtensionEmblemManager::requestClearCache);

    bool published = dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl,
                                                  quint64(7), QUrl::fromLocalFile("/tmp/emblem_dir"));

    EXPECT_TRUE(published);
    EXPECT_GE(spy.count(), 1);
}

// ========== AppendCompressEventReceiver: drag & drop hooks ==========

TEST_F(UtilsEventDispatchTest, DragMoveHook_RunWorkspaceUrls_DropActionPointerRoundTrips)
{
    stub.set_lamda(ADDR(AppendCompressHelper, setMouseStyle),
                   [](const QUrl &toUrl, const QList<QUrl> &fromUrls, Qt::DropAction *dropAction) -> bool {
                       g_mouseStyleCalled = true;
                       g_mouseToUrl = toUrl;
                       g_mouseFromUrls = fromUrls;
                       if (dropAction)
                           *dropAction = Qt::CopyAction;
                       return true;
                   });

    AppendCompressEventReceiver receiver;
    receiver.initEventConnect();

    QList<QUrl> fromUrls { createTestFile("drag_src.txt") };
    QUrl toUrl = QUrl::fromLocalFile(tempDir->path() + "/archive.zip");
    Qt::DropAction action = Qt::IgnoreAction;

    bool handled = dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileDragMove",
                                        fromUrls, toUrl, &action);

    EXPECT_TRUE(handled);
    ASSERT_TRUE(g_mouseStyleCalled);
    EXPECT_EQ(g_mouseToUrl, toUrl);
    EXPECT_EQ(action, Qt::CopyAction);
}

TEST_F(UtilsEventDispatchTest, FileDropHook_RunWorkspaceUrls_CompressRequestedWithExactUrls)
{
    stub.set_lamda(ADDR(AppendCompressHelper, dragDropCompress),
                   [](const QUrl &toUrl, const QList<QUrl> &fromUrls) -> bool {
                       g_compressCalled = true;
                       g_compressToUrl = toUrl;
                       g_compressFromUrls = fromUrls;
                       return true;
                   });

    AppendCompressEventReceiver receiver;
    receiver.initEventConnect();

    QList<QUrl> fromUrls { createTestFile("drop_src.txt") };
    QUrl toUrl = QUrl::fromLocalFile(tempDir->path() + "/bundle.zip");

    bool handled = dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileDrop",
                                        fromUrls, toUrl);

    EXPECT_TRUE(handled);
    ASSERT_TRUE(g_compressCalled);
    EXPECT_EQ(g_compressToUrl, toUrl);
    EXPECT_EQ(g_compressFromUrls, fromUrls);
}

TEST_F(UtilsEventDispatchTest, IsDropHook_RunCompressedUrl_ReturnsTrue)
{
    stub.set_lamda(ADDR(AppendCompressHelper, isCompressedFile),
                   [](const QUrl &) -> bool {
                       g_compressIsDrop = true;
                       return true;
                   });

    AppendCompressEventReceiver receiver;
    receiver.initEventConnect();

    bool handled = dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_IsDrop",
                                        QUrl::fromLocalFile("/tmp/archive.zip"));

    EXPECT_TRUE(handled);
    ASSERT_TRUE(g_compressIsDrop);
}

TEST_F(UtilsEventDispatchTest, IsDropHook_RunPlainUrl_ReturnsFalse)
{
    stub.set_lamda(ADDR(AppendCompressHelper, isCompressedFile),
                   [](const QUrl &) -> bool { return false; });

    AppendCompressEventReceiver receiver;
    receiver.initEventConnect();

    bool handled = dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_IsDrop",
                                        QUrl::fromLocalFile("/tmp/notes.txt"));

    EXPECT_FALSE(handled);
}

TEST_F(UtilsEventDispatchTest, CanvasDragMoveHook_RunDesktopPayload_MouseStyleApplied)
{
    stub.set_lamda(ADDR(AppendCompressHelper, setMouseStyle),
                   [](const QUrl &toUrl, const QList<QUrl> &fromUrls, Qt::DropAction *dropAction) -> bool {
                       g_mouseStyleCalled = true;
                       g_mouseToUrl = toUrl;
                       g_mouseFromUrls = fromUrls;
                       if (dropAction)
                           *dropAction = Qt::MoveAction;
                       return true;
                   });

    AppendCompressEventReceiver receiver;
    receiver.initEventConnect();

    QMimeData mime;
    QList<QUrl> fromUrls { createTestFile("canvas_drag.txt") };
    mime.setUrls(fromUrls);
    QUrl hoverUrl = QUrl::fromLocalFile(tempDir->path() + "/canvas_dir");
    Qt::DropAction action = Qt::IgnoreAction;

    QVariantHash extData;
    extData.insert("hoverUrl", hoverUrl);
    extData.insert("dropAction", static_cast<qlonglong>(reinterpret_cast<qintptr>(&action)));

    bool handled = dpfHookSequence->run("ddplugin_canvas", "hook_CanvasView_DragMove",
                                        3, &mime, QPoint(10, 20),
                                        static_cast<void *>(&extData));

    EXPECT_TRUE(handled);
    ASSERT_TRUE(g_mouseStyleCalled);
    EXPECT_EQ(g_mouseToUrl, hoverUrl);
    EXPECT_EQ(action, Qt::MoveAction);
}

TEST_F(UtilsEventDispatchTest, CanvasDropDataHook_RunDesktopPayload_CompressApplied)
{
    stub.set_lamda(ADDR(AppendCompressHelper, dragDropCompress),
                   [](const QUrl &toUrl, const QList<QUrl> &fromUrls) -> bool {
                       g_compressCalled = true;
                       g_compressToUrl = toUrl;
                       g_compressFromUrls = fromUrls;
                       return true;
                   });

    AppendCompressEventReceiver receiver;
    receiver.initEventConnect();

    QMimeData mime;
    QList<QUrl> fromUrls { createTestFile("canvas_drop.txt") };
    mime.setUrls(fromUrls);
    QUrl dropUrl = QUrl::fromLocalFile(tempDir->path() + "/canvas_bundle.zip");

    QVariantHash extData;
    extData.insert("dropUrl", dropUrl);

    bool handled = dpfHookSequence->run("ddplugin_canvas", "hook_CanvasView_DropData",
                                        3, &mime, QPoint(5, 6),
                                        static_cast<void *>(&extData));

    EXPECT_TRUE(handled);
    ASSERT_TRUE(g_compressCalled);
    EXPECT_EQ(g_compressToUrl, dropUrl);
    EXPECT_EQ(g_compressFromUrls, fromUrls);
}

TEST_F(UtilsEventDispatchTest, OrganizerDragMoveHook_RunOrganizerPayload_MouseStyleApplied)
{
    stub.set_lamda(ADDR(AppendCompressHelper, setMouseStyle),
                   [](const QUrl &toUrl, const QList<QUrl> &fromUrls, Qt::DropAction *dropAction) -> bool {
                       g_mouseStyleCalled = true;
                       g_mouseToUrl = toUrl;
                       g_mouseFromUrls = fromUrls;
                       if (dropAction)
                           *dropAction = Qt::LinkAction;
                       return true;
                   });

    AppendCompressEventReceiver receiver;
    receiver.initEventConnect();

    QMimeData mime;
    QList<QUrl> fromUrls { createTestFile("org_drag.txt") };
    mime.setUrls(fromUrls);
    QUrl hoverUrl = QUrl::fromLocalFile(tempDir->path() + "/organizer_dir");
    Qt::DropAction action = Qt::IgnoreAction;

    QVariantHash extData;
    extData.insert("hoverUrl", hoverUrl);
    extData.insert("dropAction", static_cast<qlonglong>(reinterpret_cast<qintptr>(&action)));

    bool handled = dpfHookSequence->run("ddplugin_organizer", "hook_CollectionView_DragMove",
                                        QString("view-7"), &mime, QPoint(30, 40),
                                        static_cast<void *>(&extData));

    EXPECT_TRUE(handled);
    ASSERT_TRUE(g_mouseStyleCalled);
    EXPECT_EQ(g_mouseToUrl, hoverUrl);
    EXPECT_EQ(action, Qt::LinkAction);
}

TEST_F(UtilsEventDispatchTest, OrganizerDropDataHook_RunOrganizerPayload_CompressApplied)
{
    stub.set_lamda(ADDR(AppendCompressHelper, dragDropCompress),
                   [](const QUrl &toUrl, const QList<QUrl> &fromUrls) -> bool {
                       g_compressCalled = true;
                       g_compressToUrl = toUrl;
                       g_compressFromUrls = fromUrls;
                       return true;
                   });

    AppendCompressEventReceiver receiver;
    receiver.initEventConnect();

    QMimeData mime;
    QList<QUrl> fromUrls { createTestFile("org_drop.txt") };
    mime.setUrls(fromUrls);
    QUrl dropUrl = QUrl::fromLocalFile(tempDir->path() + "/organizer_bundle.zip");

    QVariantHash extData;
    extData.insert("dropUrl", dropUrl);

    bool handled = dpfHookSequence->run("ddplugin_organizer", "hook_CollectionView_DropData",
                                        QString("view-9"), &mime, QPoint(50, 60),
                                        static_cast<void *>(&extData));

    EXPECT_TRUE(handled);
    ASSERT_TRUE(g_compressCalled);
    EXPECT_EQ(g_compressToUrl, dropUrl);
    EXPECT_EQ(g_compressFromUrls, fromUrls);
    EXPECT_EQ(g_compressFromUrls.size(), 1);
}

#include "test_realevents_dispatch.moc"
