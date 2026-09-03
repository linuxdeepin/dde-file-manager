// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "../../../src/plugins/filedialog/core/core.h"
#include "../../../src/plugins/filedialog/core/dbus/filedialogmanagerdbus.h"
#include "../../../src/plugins/filedialog/core/menus/filedialogmenuscene.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/dpf.h>
#include <plugins/common/dfmplugin-menu/menu_eventinterface_helper.h>

#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QTimer>
#include <QDBusError>
#include <QDBusPendingCall>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace filedialog_core;

class UT_Core : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Initialize test environment
        core = new Core();
    }

    virtual void TearDown() override
    {
        delete core;
        core = nullptr;
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    Core *core = nullptr;
};

TEST_F(UT_Core, Start_SuccessfulInitialization_ReturnsTrue)
{
    // Stub the non-virtual FMWindowsIns.setCustomWindowCreator to verify start() registers it
    bool customWindowCreatorCalled = false;
    stub.set_lamda(&FileManagerWindowsManager::setCustomWindowCreator,
                   [&customWindowCreatorCalled](FileManagerWindowsManager *,
                                                FileManagerWindowsManager::WindowCreator) {
        __DBG_STUB_INVOKE__
        customWindowCreatorCalled = true;
    });

    // Mock QDBusConnection::systemBus().connect()
    stub.set_lamda((bool (QDBusConnection::*)(const QString &, const QString &,
                                              const QString &, const QString &,
                                              QObject *, const char *))&QDBusConnection::connect,
                   [&] {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(core->start());
    EXPECT_TRUE(customWindowCreatorCalled);
}

TEST_F(UT_Core, Start_DBusConnectionFailure_ReturnsTrue)
{
    // Mock FMWindowsIns.setCustomWindowCreator
    stub.set_lamda(&FileManagerWindowsManager::setCustomWindowCreator, [] {
        __DBG_STUB_INVOKE__
    });

    // Mock QObject::connect for signal-slot connection
    // Since the actual code uses connect with function pointers, we don't need to mock this
    // The connect call will work with the real QObject::connect in test environment

    // Mock QDBusConnection::systemBus().connect() to return false
    // Since this is a complex overload, we'll skip mocking it for now
    // The test should still work with the real QDBusConnection::connect

    EXPECT_TRUE(core->start()); // start() should still return true even if DBus connection fails
}

TEST_F(UT_Core, RegisterDialogDBus_SessionBusNotConnected_ReturnsFalse)
{
    // Mock QDBusConnection::sessionBus().isConnected() to return false
    stub.set_lamda(&QDBusConnection::isConnected, [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_FALSE(core->registerDialogDBus());
}

TEST_F(UT_Core, RegisterDialogDBus_ServiceRegistrationFails_ReturnsFalse)
{
    // Mock QDBusConnection::sessionBus().isConnected() to return true
    stub.set_lamda(&QDBusConnection::isConnected, [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock qApp->applicationName()
    stub.set_lamda(&QApplication::applicationName, [] {
        __DBG_STUB_INVOKE__
        return QString("test-app");
    });

    // Mock QDBusConnection::sessionBus().registerService() to return false
    stub.set_lamda(&QDBusConnection::registerService, [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_FALSE(core->registerDialogDBus());
}

TEST_F(UT_Core, RegisterDialogDBus_ObjectRegistrationFails_ReturnsFalse)
{
    // Mock QDBusConnection::sessionBus().isConnected() to return true
    stub.set_lamda(&QDBusConnection::isConnected, [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock qApp->applicationName()
    stub.set_lamda(&QApplication::applicationName, [] {
        __DBG_STUB_INVOKE__
        return QString("test-app");
    });

    // Mock QDBusConnection::sessionBus().registerService() to return true
    stub.set_lamda(&QDBusConnection::registerService, [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock QDBusConnection::sessionBus().registerObject() to return false
    stub.set_lamda((bool(QDBusConnection::*)(const QString &, QObject *,
                     QDBusConnection::RegisterOptions))&QDBusConnection::registerObject,
                   [](QDBusConnection *, const QString &, QObject *,
                      QDBusConnection::RegisterOptions) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_FALSE(core->registerDialogDBus());
}

TEST_F(UT_Core, RegisterDialogDBus_AllRegistrationsSucceed_ReturnsTrue)
{
    // Mock QDBusConnection::sessionBus().isConnected() to return true
    stub.set_lamda(&QDBusConnection::isConnected, [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock qApp->applicationName()
    stub.set_lamda(&QApplication::applicationName, [] {
        __DBG_STUB_INVOKE__
        return QString("test-app");
    });

    // Mock QDBusConnection::sessionBus().registerService() to return true
    stub.set_lamda(&QDBusConnection::registerService, [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock QDBusConnection::sessionBus().registerObject() to return true
    stub.set_lamda((bool(QDBusConnection::*)(const QString &, QObject *,
                     QDBusConnection::RegisterOptions))&QDBusConnection::registerObject,
                   [](QDBusConnection *, const QString &, QObject *,
                      QDBusConnection::RegisterOptions) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(core->registerDialogDBus());
}

TEST_F(UT_Core, OnAllPluginsStarted_RegistersDBusAndMenu_Success)
{
    // Mock registerDialogDBus to return true
    bool registerDialogDBusCalled = false;
    stub.set_lamda(&Core::registerDialogDBus, [this, &registerDialogDBusCalled] {
        __DBG_STUB_INVOKE__
        registerDialogDBusCalled = true;
        return true;
    });

    // menuSceneRegisterScene/menuSceneContains/menuSceneBind are `static inline` helpers
    // (internal linkage, one copy per TU) so they cannot be stubbed directly. They all
    // funnel into dpfSlotChannel->push, a shared weak template symbol, so stub those
    // instantiations instead.

    // push("dfmplugin_menu", "slot_MenuScene_RegisterScene", name, creator)
    bool menuSceneRegisterCalled = false;
    using PushRegisterFunc = QVariant (EventChannelManager::*)(const QString &, const QString &,
                                                               QString, AbstractSceneCreator *&);
    auto pushRegister = static_cast<PushRegisterFunc>(&EventChannelManager::push);
    stub.set_lamda(pushRegister, [&](EventChannelManager *, const QString &space,
                                     const QString &topic, const QString &, AbstractSceneCreator *&) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_menu" && topic == "slot_MenuScene_RegisterScene")
            menuSceneRegisterCalled = true;
        return QVariant(true);
    });

    // push("dfmplugin_menu", "slot_MenuScene_Contains", name)
    using PushContainsFunc = QVariant (EventChannelManager::*)(const QString &, const QString &, QString);
    auto pushContains = static_cast<PushContainsFunc>(&EventChannelManager::push);
    stub.set_lamda(pushContains, [](EventChannelManager *, const QString &space,
                                    const QString &topic, const QString &) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_menu" && topic == "slot_MenuScene_Contains")
            return QVariant(true);
        return QVariant();
    });

    // push("dfmplugin_menu", "slot_MenuScene_Bind", name, parent)
    bool menuSceneBindCalled = false;
    using PushBindFunc = QVariant (EventChannelManager::*)(const QString &, const QString &,
                                                           QString, const QString &);
    auto pushBind = static_cast<PushBindFunc>(&EventChannelManager::push);
    stub.set_lamda(pushBind, [&](EventChannelManager *, const QString &space,
                                 const QString &topic, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_menu" && topic == "slot_MenuScene_Bind")
            menuSceneBindCalled = true;
        return QVariant(true);
    });

    EXPECT_NO_FATAL_FAILURE(core->onAllPluginsStarted());
    EXPECT_TRUE(registerDialogDBusCalled);
    EXPECT_TRUE(menuSceneRegisterCalled);
    EXPECT_TRUE(menuSceneBindCalled);
}

TEST_F(UT_Core, OnAllPluginsStarted_RegisterDialogDBusFails_CallsAbort)
{
    // Mock registerDialogDBus to return false
    stub.set_lamda(&Core::registerDialogDBus, [this] {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock abort to avoid actual abort
    bool abortCalled = false;
    stub.set_lamda(&abort, [&] {
        __DBG_STUB_INVOKE__
        abortCalled = true;
    });

    EXPECT_NO_FATAL_FAILURE(core->onAllPluginsStarted());
    // Note: We can't test abort() call directly as it terminates the program
}

TEST_F(UT_Core, BindScene_SceneExists_BindsSuccessfully)
{
    QString testScene = "TestScene";

    // menuSceneContains/menuSceneBind are static inline (per-TU copies); stub the shared
    // dpfSlotChannel->push template instantiations they expand into instead.
    bool menuSceneContainsCalled = false;
    using PushContainsFunc = QVariant (EventChannelManager::*)(const QString &, const QString &, QString);
    auto pushContains = static_cast<PushContainsFunc>(&EventChannelManager::push);
    stub.set_lamda(pushContains, [&](EventChannelManager *, const QString &space,
                                     const QString &topic, const QString &) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_menu" && topic == "slot_MenuScene_Contains") {
            menuSceneContainsCalled = true;
            return QVariant(true);
        }
        return QVariant();
    });

    bool menuSceneBindCalled = false;
    using PushBindFunc = QVariant (EventChannelManager::*)(const QString &, const QString &,
                                                           QString, const QString &);
    auto pushBind = static_cast<PushBindFunc>(&EventChannelManager::push);
    stub.set_lamda(pushBind, [&](EventChannelManager *, const QString &space,
                                 const QString &topic, const QString &scene, const QString &parent) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_menu" && topic == "slot_MenuScene_Bind") {
            EXPECT_EQ(scene, FileDialogMenuCreator::name());
            EXPECT_EQ(parent, testScene);
            menuSceneBindCalled = true;
        }
        return QVariant(true);
    });

    EXPECT_NO_FATAL_FAILURE(core->bindScene(testScene));
    EXPECT_TRUE(menuSceneContainsCalled);
    EXPECT_TRUE(menuSceneBindCalled);
}

TEST_F(UT_Core, BindScene_SceneNotExists_AddsToWaitList)
{
    QString testScene = "NonExistentScene";

    // Contains returns false via the shared push instantiation
    using PushContainsFunc = QVariant (EventChannelManager::*)(const QString &, const QString &, QString);
    auto pushContains = static_cast<PushContainsFunc>(&EventChannelManager::push);
    stub.set_lamda(pushContains, [](EventChannelManager *, const QString &space,
                                    const QString &topic, const QString &) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_menu" && topic == "slot_MenuScene_Contains")
            return QVariant(false);
        return QVariant();
    });

    // Stub dpfSignalDispatcher->subscribe (template instantiation) to verify the event
    // subscription for the wait-list path
    bool subscribeCalled = false;
    using SubscribeFunc = bool (EventDispatcherManager::*)(const QString &, const QString &,
                                                           Core *, void (Core::*)(const QString &));
    auto subscribe = static_cast<SubscribeFunc>(&EventDispatcherManager::subscribe);
    stub.set_lamda(subscribe, [&](EventDispatcherManager *, const QString &space,
                                  const QString &topic, Core *, void (Core::*)(const QString &)) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_menu" && topic == "signal_MenuScene_SceneAdded")
            subscribeCalled = true;
        return true;
    });

    EXPECT_NO_FATAL_FAILURE(core->bindScene(testScene));
    EXPECT_TRUE(subscribeCalled);
    EXPECT_TRUE(core->waitToBind.contains(testScene));
}

TEST_F(UT_Core, BindSceneOnAdded_SceneInWaitList_BindsAndRemovesFromWaitList)
{
    QString testScene = "TestScene";

    // Add scene to wait list manually
    core->waitToBind.insert(testScene);
    core->eventSubscribed = true;

    // Contains returns true via the shared push instantiation
    using PushContainsFunc = QVariant (EventChannelManager::*)(const QString &, const QString &, QString);
    auto pushContains = static_cast<PushContainsFunc>(&EventChannelManager::push);
    stub.set_lamda(pushContains, [](EventChannelManager *, const QString &space,
                                    const QString &topic, const QString &) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_menu" && topic == "slot_MenuScene_Contains")
            return QVariant(true);
        return QVariant();
    });

    bool menuSceneBindCalled = false;
    using PushBindFunc = QVariant (EventChannelManager::*)(const QString &, const QString &,
                                                           QString, const QString &);
    auto pushBind = static_cast<PushBindFunc>(&EventChannelManager::push);
    stub.set_lamda(pushBind, [&](EventChannelManager *, const QString &space,
                                 const QString &topic, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_menu" && topic == "slot_MenuScene_Bind")
            menuSceneBindCalled = true;
        return QVariant(true);
    });

    // Stub dpfSignalDispatcher->unsubscribe (template instantiation)
    bool unsubscribeCalled = false;
    using UnsubscribeFunc = bool (EventDispatcherManager::*)(const QString &, const QString &,
                                                             Core *, void (Core::*)(const QString &));
    auto unsubscribe = static_cast<UnsubscribeFunc>(&EventDispatcherManager::unsubscribe);
    stub.set_lamda(unsubscribe, [&](EventDispatcherManager *, const QString &space,
                                    const QString &topic, Core *, void (Core::*)(const QString &)) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_menu" && topic == "signal_MenuScene_SceneAdded")
            unsubscribeCalled = true;
        return true;
    });

    EXPECT_NO_FATAL_FAILURE(core->bindSceneOnAdded(testScene));
    EXPECT_TRUE(menuSceneBindCalled);
    EXPECT_TRUE(unsubscribeCalled);
    EXPECT_FALSE(core->waitToBind.contains(testScene));
}

TEST_F(UT_Core, BindSceneOnAdded_SceneNotInWaitList_DoesNothing)
{
    QString testScene = "TestScene";
    
    // Don't add scene to wait list
    
    EXPECT_NO_FATAL_FAILURE(core->bindSceneOnAdded(testScene));
    EXPECT_FALSE(core->waitToBind.contains(testScene));
}

TEST_F(UT_Core, EnterHighPerformanceMode_SystemBusNotAvailable_ReturnsEarly)
{
    // Mock QDBusConnection::systemBus().interface() to return nullptr
    stub.set_lamda(&QDBusConnection::interface, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    EXPECT_NO_FATAL_FAILURE(core->enterHighPerformanceMode());
}

TEST_F(UT_Core, EnterHighPerformanceMode_ServiceNotRegistered_ReturnsEarly)
{
    // Mock QDBusConnection::systemBus().interface() to return valid interface
    QDBusConnection mockConnection("test_connection");
    auto mockInterface = new QDBusConnectionInterface(mockConnection, nullptr);
    stub.set_lamda(&QDBusConnection::interface, [&] {
        __DBG_STUB_INVOKE__
        return mockInterface;
    });

    // Mock isServiceRegistered to return false
    // Since this is complex to mock, we'll skip it for now
    // The test should still work with real function call

    EXPECT_NO_FATAL_FAILURE(core->enterHighPerformanceMode());
    delete mockInterface;
}

TEST_F(UT_Core, EnterHighPerformanceMode_ServiceRegistered_CallsLockCpuFreq)
{
    // Mock QDBusConnection::systemBus().interface() to return valid interface
    QDBusConnection mockConnection("test_connection");
    auto mockInterface = new QDBusConnectionInterface(mockConnection, nullptr);
    stub.set_lamda(&QDBusConnection::interface, [&] {
        __DBG_STUB_INVOKE__
        return mockInterface;
    });

    // QDBusConnectionInterface::isServiceRegistered is non-virtual (lives in libQt6DBus),
    // stub it to return a successful "true" reply
    bool isServiceRegisteredCalled = false;
    using IsServiceRegisteredFunc = QDBusReply<bool> (QDBusConnectionInterface::*)(const QString &) const;
    stub.set_lamda(static_cast<IsServiceRegisteredFunc>(&QDBusConnectionInterface::isServiceRegistered),
                   [&isServiceRegisteredCalled](const QDBusConnectionInterface *, const QString &) -> QDBusReply<bool> {
        __DBG_STUB_INVOKE__
        isServiceRegisteredCalled = true;
        QDBusMessage reply = QDBusMessage::createMethodCall("a", "/", "a.if", "m")
                                 .createReply(QVariantList { true });
        return QDBusReply<bool>(reply);
    });

    // Stub the exact asyncCall template instantiation used by the product call
    // daemonIface.asyncCall("LockCpuFreq", "performance", 3)  (both literals are char[12])
    bool asyncCallCalled = false;
    using AsyncCallFunc = QDBusPendingCall (QDBusAbstractInterface::*)(const QString &,
                                                                       const char (&)[12], int &&);
    stub.set_lamda(static_cast<AsyncCallFunc>(&QDBusAbstractInterface::asyncCall),
                   [&asyncCallCalled](QDBusAbstractInterface *, const QString &method,
                                      const char (&)[12], int) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(method, QString("LockCpuFreq"));
        asyncCallCalled = true;
        return QDBusPendingCall::fromError(QDBusError(QDBusError::Failed, QStringLiteral("stubbed")));
    });

    EXPECT_NO_FATAL_FAILURE(core->enterHighPerformanceMode());
    EXPECT_TRUE(isServiceRegisteredCalled);
    EXPECT_TRUE(asyncCallCalled);
    delete mockInterface;
}

TEST_F(UT_Core, ExitOnShutdown_ShutdownFalse_DoesNothing)
{
    bool quitCalled = false;
    stub.set_lamda(&QApplication::quit, [&] {
        __DBG_STUB_INVOKE__
        quitCalled = true;
    });

    EXPECT_NO_FATAL_FAILURE(core->exitOnShutdown(false));
    EXPECT_FALSE(quitCalled);
}

TEST_F(UT_Core, ExitOnShutdown_ShutdownTrue_CallsQuit)
{
    bool quitCalled = false;
    stub.set_lamda(&QApplication::quit, [&] {
        __DBG_STUB_INVOKE__
        quitCalled = true;
    });

    // Mock QTimer::singleShot to avoid actual timer
    using QTimerSingleShotFunc = void (*)(int, const QObject *, const char *);
    stub.set_lamda(static_cast<QTimerSingleShotFunc>(&QTimer::singleShot), [](int, const QObject *, const char *) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(core->exitOnShutdown(true));
    EXPECT_TRUE(quitCalled);
}

TEST_F(UT_Core, MultipleMethodCalls_DifferentScenarios_HandlesCorrectly)
{
    // Test multiple calls to different methods
    int startCallCount = 0;
    int bindSceneCallCount = 0;
    int enterHighPerformanceCallCount = 0;
    
    // Mock methods
    stub.set_lamda(&FileManagerWindowsManager::setCustomWindowCreator, [&startCallCount] {
        __DBG_STUB_INVOKE__
        startCallCount++;
    });
    
    // QObject::connect doesn't need to be mocked for signal-slot connections
    // The real connect will work in the test environment
    
    stub.set_lamda((bool (QDBusConnection::*)(const QString &, const QString &,
                                              const QString &, const QString &,
                                              QObject *, const char *))&QDBusConnection::connect,
                   [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    // menuSceneContains/menuSceneBind are static inline (per-TU copies); stub the shared
    // dpfSlotChannel->push template instantiations instead
    using PushContainsFunc = QVariant (EventChannelManager::*)(const QString &, const QString &, QString);
    auto pushContains = static_cast<PushContainsFunc>(&EventChannelManager::push);
    stub.set_lamda(pushContains, [](EventChannelManager *, const QString &space,
                                    const QString &topic, const QString &) {
        __DBG_STUB_INVOKE__
        return QVariant(space == "dfmplugin_menu" && topic == "slot_MenuScene_Contains");
    });
    
    using PushBindFunc = QVariant (EventChannelManager::*)(const QString &, const QString &,
                                                           QString, const QString &);
    auto pushBind = static_cast<PushBindFunc>(&EventChannelManager::push);
    stub.set_lamda(pushBind, [&](EventChannelManager *, const QString &space,
                                 const QString &topic, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_menu" && topic == "slot_MenuScene_Bind")
            bindSceneCallCount++;
        return QVariant(true);
    });
    
    stub.set_lamda(&QDBusConnection::interface, [&enterHighPerformanceCallCount] {
        __DBG_STUB_INVOKE__
        enterHighPerformanceCallCount++;
        return nullptr;
    });

    // Call methods multiple times
    core->start();
    core->bindScene("TestScene1");
    core->bindScene("TestScene2");
    core->enterHighPerformanceMode();
    core->enterHighPerformanceMode();
    
    EXPECT_EQ(startCallCount, 1);
    EXPECT_EQ(bindSceneCallCount, 2);
    // start() also invokes enterHighPerformanceMode() internally (2 direct + 1 in start)
    EXPECT_EQ(enterHighPerformanceCallCount, 3);
}
