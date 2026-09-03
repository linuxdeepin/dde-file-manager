// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define private public
#define protected public
#include "abstractindexcontroller.h"
#include "textindex_interface.h"
#undef private
#undef protected

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QDBusAbstractInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDir>
#include <QTimer>

DAEMONPCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace {

QDBusMessage makeReply(const QVariant &value = QVariant(true))
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
            QStringLiteral("test.service"),
            QStringLiteral("/test"),
            QStringLiteral("test.interface"),
            QStringLiteral("test.method"));
    return msg.createReply(value);
}


IndexControllerDescriptor buildTestDescriptor(const std::function<QStringList()> &provider = nullptr)
{
    return IndexControllerDescriptor {
        QStringLiteral("TestController"),
        QStringLiteral("org.deepin.Filemanager.TextIndex"),
        QStringLiteral("/org/deepin/Filemanager/TextIndex"),
        QStringLiteral("org.deepin.dde.file-manager.search"),
        QStringLiteral("enableFullTextSearch"),
        provider,
        [](QObject *parent) -> QDBusAbstractInterface * {
            return new OrgDeepinFilemanagerTextIndexInterface(
                    QStringLiteral("org.deepin.Filemanager.TextIndex"),
                    QStringLiteral("/org/deepin/Filemanager/TextIndex"),
                    QDBusConnection(QStringLiteral("test")),
                    parent);
        }
    };
}

IndexControllerDescriptor buildDescriptorWithoutFactory()
{
    IndexControllerDescriptor descriptor = buildTestDescriptor();
    descriptor.interfaceFactory = nullptr;
    return descriptor;
}

}   // namespace

class AbstractIndexControllerImpl : public testing::Test
{
protected:
    void TearDown() override
    {
        delete controller;
        stub.clear();
    }

    AbstractIndexController *controller { nullptr };
    stub_ext::StubExt stub;
};

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

TEST_F(AbstractIndexControllerImpl, Constructor_InitializesStateAndTimer)
{
    controller = new AbstractIndexController(buildTestDescriptor());

    EXPECT_EQ(controller->currentState, AbstractIndexController::State::Disabled);
    EXPECT_NE(controller->keepAliveTimer, nullptr);
    EXPECT_FALSE(controller->isConfigEnabled);
}

TEST_F(AbstractIndexControllerImpl, Destructor_CleansUp)
{
    controller = new AbstractIndexController(buildTestDescriptor());
    delete controller;
    controller = nullptr;

    EXPECT_TRUE(true);
}

// ---------------------------------------------------------------------------
// initialize
// ---------------------------------------------------------------------------

TEST_F(AbstractIndexControllerImpl, Initialize_ConfigRegistrationFailed_ReturnsEarly)
{
    bool valueCalled = false;

    stub.set_lamda(&DConfigManager::instance, []() {
        __DBG_STUB_INVOKE__
        static DConfigManager manager;
        return &manager;
    });

    stub.set_lamda(&DConfigManager::addConfig, [&](DConfigManager *, const QString &name, QString *err) {
        __DBG_STUB_INVOKE__
        if (err) *err = QStringLiteral("registration failed");
        return false;
    });

    stub.set_lamda(&DConfigManager::value, [&](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        valueCalled = true;
        return QVariant();
    });

    controller = new AbstractIndexController(buildTestDescriptor());
    controller->initialize();

    EXPECT_FALSE(valueCalled);
}

TEST_F(AbstractIndexControllerImpl, Initialize_ConfigDisabled_DoesNotActivateBackend)
{
    bool activeBackendCalled = false;

    stub.set_lamda(&DConfigManager::instance, []() {
        __DBG_STUB_INVOKE__
        static DConfigManager manager;
        return &manager;
    });

    stub.set_lamda(&DConfigManager::addConfig, [&](DConfigManager *, const QString &, QString *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DConfigManager::value, [&](DConfigManager *, const QString &config, const QString &key, const QVariant &) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(config, QStringLiteral("org.deepin.dde.file-manager.search"));
        EXPECT_EQ(key, QStringLiteral("enableFullTextSearch"));
        return QVariant(false);
    });

    stub.set_lamda(&AbstractIndexController::activeBackend, [&](AbstractIndexController *, bool) {
        __DBG_STUB_INVOKE__
        activeBackendCalled = true;
    });

    controller = new AbstractIndexController(buildTestDescriptor());
    controller->initialize();

    EXPECT_FALSE(activeBackendCalled);
    EXPECT_FALSE(controller->isConfigEnabled);
}

TEST_F(AbstractIndexControllerImpl, Initialize_ConfigEnabled_ActivatesBackend)
{
    bool activeBackendCalled = false;
    bool updateTimerCalled = false;

    stub.set_lamda(&DConfigManager::instance, []() {
        __DBG_STUB_INVOKE__
        static DConfigManager manager;
        return &manager;
    });

    stub.set_lamda(&DConfigManager::addConfig, [&](DConfigManager *, const QString &, QString *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DConfigManager::value, [&](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });

    stub.set_lamda(&AbstractIndexController::activeBackend, [&](AbstractIndexController *, bool isInit) {
        __DBG_STUB_INVOKE__
        activeBackendCalled = true;
        EXPECT_TRUE(isInit);
    });

    stub.set_lamda(&AbstractIndexController::updateKeepAliveTimer, [&](AbstractIndexController *) {
        __DBG_STUB_INVOKE__
        updateTimerCalled = true;
    });

    controller = new AbstractIndexController(buildTestDescriptor());
    controller->initialize();

    EXPECT_TRUE(activeBackendCalled);
    EXPECT_TRUE(updateTimerCalled);
    EXPECT_TRUE(controller->isConfigEnabled);
}

// ---------------------------------------------------------------------------
// indexedPaths / descriptor
// ---------------------------------------------------------------------------

TEST_F(AbstractIndexControllerImpl, IndexedPaths_UsesProvider)
{
    QStringList expected { QStringLiteral("/path/one"), QStringLiteral("/path/two") };

    controller = new AbstractIndexController(buildTestDescriptor([&expected]() { return expected; }));

    EXPECT_EQ(controller->indexedPaths(), expected);
}

TEST_F(AbstractIndexControllerImpl, IndexedPaths_NoProvider_ReturnsEmpty)
{
    controller = new AbstractIndexController(buildDescriptorWithoutFactory());

    EXPECT_TRUE(controller->indexedPaths().isEmpty());
}

TEST_F(AbstractIndexControllerImpl, Descriptor_ReturnsReference)
{
    controller = new AbstractIndexController(buildTestDescriptor());

    const IndexControllerDescriptor &desc = controller->descriptor();
    EXPECT_EQ(desc.controllerName, QStringLiteral("TestController"));
    EXPECT_EQ(desc.configPath, QStringLiteral("org.deepin.dde.file-manager.search"));
}

// ---------------------------------------------------------------------------
// updateState
// ---------------------------------------------------------------------------

TEST_F(AbstractIndexControllerImpl, UpdateState_TransitionsState)
{
    controller = new AbstractIndexController(buildTestDescriptor());

    controller->updateState(AbstractIndexController::State::Idle);
    EXPECT_EQ(controller->currentState, AbstractIndexController::State::Idle);

    controller->updateState(AbstractIndexController::State::Running);
    EXPECT_EQ(controller->currentState, AbstractIndexController::State::Running);

    controller->updateState(AbstractIndexController::State::Running);
    EXPECT_EQ(controller->currentState, AbstractIndexController::State::Running);
}

// ---------------------------------------------------------------------------
// onTaskFinished
// ---------------------------------------------------------------------------

TEST_F(AbstractIndexControllerImpl, OnTaskFinished_NotTracked_DoesNothing)
{
    controller = new AbstractIndexController(buildTestDescriptor([&]() {
        return QStringList { QStringLiteral("/tracked/path") };
    }));
    controller->updateState(AbstractIndexController::State::Running);

    // Call with an untracked path.
    controller->onTaskFinished(QStringLiteral("create"), QStringLiteral("/other/path"), true);

    // State should stay Running because no handler was invoked.
    EXPECT_EQ(controller->currentState, AbstractIndexController::State::Running);
}

TEST_F(AbstractIndexControllerImpl, OnTaskFinished_TrackedRunningSuccess_TransitionsToIdle)
{
    QStringList paths { QStringLiteral("/tracked/path") };
    controller = new AbstractIndexController(buildTestDescriptor([&]() { return paths; }));
    controller->updateState(AbstractIndexController::State::Running);

    controller->onTaskFinished(QStringLiteral("create"), QStringLiteral("/tracked/path"), true);

    EXPECT_EQ(controller->currentState, AbstractIndexController::State::Idle);
}

TEST_F(AbstractIndexControllerImpl, OnTaskFinished_TrackedRunningFailure_TransitionsToDisabled)
{
    QStringList paths { QStringLiteral("/tracked/path") };
    controller = new AbstractIndexController(buildTestDescriptor([&]() { return paths; }));
    controller->updateState(AbstractIndexController::State::Running);

    controller->onTaskFinished(QStringLiteral("create"), QStringLiteral("/tracked/path"), false);

    EXPECT_EQ(controller->currentState, AbstractIndexController::State::Disabled);
}

// ---------------------------------------------------------------------------
// onTaskProgressChanged
// ---------------------------------------------------------------------------

TEST_F(AbstractIndexControllerImpl, OnTaskProgressChanged_NotTracked_DoesNothing)
{
    controller = new AbstractIndexController(buildTestDescriptor([&]() {
        return QStringList { QStringLiteral("/tracked/path") };
    }));

    controller->onTaskProgressChanged(QStringLiteral("create"), QStringLiteral("/other/path"), 1, 10);

    EXPECT_EQ(controller->currentState, AbstractIndexController::State::Disabled);
}

TEST_F(AbstractIndexControllerImpl, OnTaskProgressChanged_TrackedAndRunning_DoesNothing)
{
    QStringList paths { QStringLiteral("/tracked/path") };
    controller = new AbstractIndexController(buildTestDescriptor([&]() { return paths; }));
    controller->updateState(AbstractIndexController::State::Running);

    controller->onTaskProgressChanged(QStringLiteral("create"), QStringLiteral("/tracked/path"), 1, 10);

    EXPECT_EQ(controller->currentState, AbstractIndexController::State::Running);
}

TEST_F(AbstractIndexControllerImpl, OnTaskProgressChanged_TrackedDisabled_StopsTaskWhenDisabled)
{
    QStringList paths { QStringLiteral("/tracked/path") };
    controller = new AbstractIndexController(buildTestDescriptor([&]() { return paths; }));

    // Provide a live interface so the stop call can be recorded.
    controller->interface.reset(new OrgDeepinFilemanagerTextIndexInterface(
            QStringLiteral("org.deepin.Filemanager.TextIndex"),
            QStringLiteral("/org/deepin/Filemanager/TextIndex"),
            QDBusConnection(QStringLiteral("test")),
            controller));

    QString capturedMethod;
    stub.set_lamda(static_cast<QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QList<QVariant> &)>(&QDBusAbstractInterface::asyncCallWithArgumentList),
                   [&](QDBusAbstractInterface *, const QString &method, const QList<QVariant> &) {
        __DBG_STUB_INVOKE__
        capturedMethod = method;
        return QDBusPendingCall::fromCompletedCall(makeReply());
    });

    controller->onTaskProgressChanged(QStringLiteral("create"), QStringLiteral("/tracked/path"), 1, 10);

    EXPECT_EQ(controller->currentState, AbstractIndexController::State::Running);
    EXPECT_EQ(capturedMethod, QStringLiteral("StopCurrentTask"));
}

// ---------------------------------------------------------------------------
// setupDBusConnections / isBackendAvaliable
// ---------------------------------------------------------------------------

TEST_F(AbstractIndexControllerImpl, SetupDBusConnections_NoFactory_LeavesInterfaceNull)
{
    controller = new AbstractIndexController(buildDescriptorWithoutFactory());

    stub.set_lamda(&QDBusConnectionInterface::startService, [](QDBusConnectionInterface *, const QString &) {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<void>();
    });

    stub.set_lamda(static_cast<QMetaObject::Connection (*)(const QObject *, const char *, const QObject *, const char *, Qt::ConnectionType)>(&QObject::connect),
                   [](const QObject *, const char *, const QObject *, const char *, Qt::ConnectionType) {
        __DBG_STUB_INVOKE__
        return QMetaObject::Connection();
    });

    controller->setupDBusConnections();

    EXPECT_TRUE(controller->interface.isNull());
}

TEST_F(AbstractIndexControllerImpl, IsBackendAvaliable_NoInterface_SetsUpInterface)
{
    controller = new AbstractIndexController(buildTestDescriptor());

    stub.set_lamda(&QDBusConnectionInterface::startService, [](QDBusConnectionInterface *, const QString &) {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<void>();
    });

    stub.set_lamda(static_cast<QMetaObject::Connection (*)(const QObject *, const char *, const QObject *, const char *, Qt::ConnectionType)>(&QObject::connect),
                   [](const QObject *, const char *, const QObject *, const char *, Qt::ConnectionType) {
        __DBG_STUB_INVOKE__
        return QMetaObject::Connection();
    });

    EXPECT_TRUE(controller->interface.isNull());
    bool result = controller->isBackendAvaliable();

    EXPECT_TRUE(result);
    EXPECT_FALSE(controller->interface.isNull());
}

TEST_F(AbstractIndexControllerImpl, IsBackendAvaliable_AlreadyAvailable_ReturnsTrue)
{
    controller = new AbstractIndexController(buildTestDescriptor());
    controller->interface.reset(new OrgDeepinFilemanagerTextIndexInterface(
            QStringLiteral("org.deepin.Filemanager.TextIndex"),
            QStringLiteral("/org/deepin/Filemanager/TextIndex"),
            QDBusConnection(QStringLiteral("test")),
            controller));

    bool result = controller->isBackendAvaliable();

    EXPECT_TRUE(result);
}

// ---------------------------------------------------------------------------
// activeBackend
// ---------------------------------------------------------------------------

TEST_F(AbstractIndexControllerImpl, ActiveBackend_NoInterface_ReturnsEarly)
{
    controller = new AbstractIndexController(buildDescriptorWithoutFactory());

    bool called = false;
    stub.set_lamda(static_cast<QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QList<QVariant> &)>(&QDBusAbstractInterface::asyncCallWithArgumentList),
                   [&](QDBusAbstractInterface *, const QString &, const QList<QVariant> &) {
        __DBG_STUB_INVOKE__
        called = true;
        return QDBusPendingCall::fromCompletedCall(makeReply());
    });

    controller->activeBackend(false);

    EXPECT_FALSE(called);
}

TEST_F(AbstractIndexControllerImpl, ActiveBackend_NotInit_SendsSetEnabled)
{
    controller = new AbstractIndexController(buildTestDescriptor());
    controller->interface.reset(new OrgDeepinFilemanagerTextIndexInterface(
            QStringLiteral("org.deepin.Filemanager.TextIndex"),
            QStringLiteral("/org/deepin/Filemanager/TextIndex"),
            QDBusConnection(QStringLiteral("test")),
            controller));
    controller->isConfigEnabled = true;

    QString capturedMethod;
    QVariant capturedArg;
    stub.set_lamda(static_cast<QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QList<QVariant> &)>(&QDBusAbstractInterface::asyncCallWithArgumentList),
                   [&](QDBusAbstractInterface *, const QString &method, const QList<QVariant> &args) {
        __DBG_STUB_INVOKE__
        capturedMethod = method;
        if (!args.isEmpty()) capturedArg = args.first();
        return QDBusPendingCall::fromCompletedCall(makeReply());
    });

    controller->activeBackend(false);

    EXPECT_EQ(capturedMethod, QStringLiteral("SetEnabled"));
    EXPECT_EQ(capturedArg.toBool(), true);
}

TEST_F(AbstractIndexControllerImpl, ActiveBackend_Init_DoesNotCrash)
{
    controller = new AbstractIndexController(buildTestDescriptor());
    controller->interface.reset(new OrgDeepinFilemanagerTextIndexInterface(
            QStringLiteral("org.deepin.Filemanager.TextIndex"),
            QStringLiteral("/org/deepin/Filemanager/TextIndex"),
            QDBusConnection(QStringLiteral("test")),
            controller));
    controller->isConfigEnabled = false;

    // The init path uses a delayed singleShot; we only need to ensure no crash.
    controller->activeBackend(true);

    EXPECT_TRUE(controller->interface);
}

// ---------------------------------------------------------------------------
// keepBackendAlive
// ---------------------------------------------------------------------------

TEST_F(AbstractIndexControllerImpl, KeepBackendAlive_NoInterface_ReturnsEarly)
{
    controller = new AbstractIndexController(buildDescriptorWithoutFactory());

    bool called = false;
    stub.set_lamda(static_cast<QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QList<QVariant> &)>(&QDBusAbstractInterface::asyncCallWithArgumentList),
                   [&](QDBusAbstractInterface *, const QString &, const QList<QVariant> &) {
        __DBG_STUB_INVOKE__
        called = true;
        return QDBusPendingCall::fromCompletedCall(makeReply());
    });

    controller->keepBackendAlive();

    EXPECT_FALSE(called);
}

TEST_F(AbstractIndexControllerImpl, KeepBackendAlive_DisabledButConfigEnabled_Reactivates)
{
    controller = new AbstractIndexController(buildTestDescriptor());
    controller->interface.reset(new OrgDeepinFilemanagerTextIndexInterface(
            QStringLiteral("org.deepin.Filemanager.TextIndex"),
            QStringLiteral("/org/deepin/Filemanager/TextIndex"),
            QDBusConnection(QStringLiteral("test")),
            controller));
    controller->isConfigEnabled = true;

    QString capturedMethod;
    stub.set_lamda(static_cast<QDBusMessage (QDBusAbstractInterface::*)(QDBus::CallMode, const QString &, const QList<QVariant> &)>(&QDBusAbstractInterface::callWithArgumentList),
                   [&](QDBusAbstractInterface *, QDBus::CallMode mode, const QString &method, const QList<QVariant> &) {
        __DBG_STUB_INVOKE__
        capturedMethod = method;
        // IsEnabled query returns false.
        if (method == QStringLiteral("IsEnabled"))
            return makeReply(QVariant(false));
        return makeReply(QVariant(true));
    });

    // Reactivation goes through activeBackend() which uses asyncCall("SetEnabled", ...);
    // in Qt 6.8 every asyncCall overload resolves to the private
    // doAsyncCall(method, args, numArgs).
    QString capturedAsyncMethod;
    using DoAsyncFunc = QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QVariant *, size_t);
    stub.set_lamda(static_cast<DoAsyncFunc>(&QDBusAbstractInterface::doAsyncCall),
                   [&](QDBusAbstractInterface *, const QString &method, const QVariant *, size_t) {
        __DBG_STUB_INVOKE__
        capturedAsyncMethod = method;
        return QDBusPendingCall::fromCompletedCall(makeReply());
    });

    controller->keepBackendAlive();

    // First call is IsEnabled; the reactivation async-calls SetEnabled.
    EXPECT_EQ(capturedMethod, QStringLiteral("IsEnabled"));
    EXPECT_EQ(capturedAsyncMethod, QStringLiteral("SetEnabled"));
}

TEST_F(AbstractIndexControllerImpl, KeepBackendAlive_Enabled_DoesNotReactivate)
{
    controller = new AbstractIndexController(buildTestDescriptor());
    controller->interface.reset(new OrgDeepinFilemanagerTextIndexInterface(
            QStringLiteral("org.deepin.Filemanager.TextIndex"),
            QStringLiteral("/org/deepin/Filemanager/TextIndex"),
            QDBusConnection(QStringLiteral("test")),
            controller));
    controller->isConfigEnabled = true;

    int callCount = 0;
    stub.set_lamda(static_cast<QDBusMessage (QDBusAbstractInterface::*)(QDBus::CallMode, const QString &, const QList<QVariant> &)>(&QDBusAbstractInterface::callWithArgumentList),
                   [&](QDBusAbstractInterface *, QDBus::CallMode, const QString &method, const QList<QVariant> &) {
        __DBG_STUB_INVOKE__
        ++callCount;
        EXPECT_EQ(method, QStringLiteral("IsEnabled"));
        return makeReply(QVariant(true));
    });

    controller->keepBackendAlive();

    EXPECT_EQ(callCount, 1);
}

// ---------------------------------------------------------------------------
// startIndexTask
// ---------------------------------------------------------------------------

TEST_F(AbstractIndexControllerImpl, StartIndexTask_NoInterface_ReturnsEarly)
{
    controller = new AbstractIndexController(buildDescriptorWithoutFactory());

    bool called = false;
    stub.set_lamda(static_cast<QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QList<QVariant> &)>(&QDBusAbstractInterface::asyncCallWithArgumentList),
                   [&](QDBusAbstractInterface *, const QString &, const QList<QVariant> &) {
        __DBG_STUB_INVOKE__
        called = true;
        return QDBusPendingCall::fromCompletedCall(makeReply());
    });

    controller->startIndexTask(true);

    EXPECT_FALSE(called);
}

TEST_F(AbstractIndexControllerImpl, StartIndexTask_Create_CallsCreateIndexTask)
{
    controller = new AbstractIndexController(buildTestDescriptor([&]() {
        return QStringList { QStringLiteral("/home/test") };
    }));
    controller->interface.reset(new OrgDeepinFilemanagerTextIndexInterface(
            QStringLiteral("org.deepin.Filemanager.TextIndex"),
            QStringLiteral("/org/deepin/Filemanager/TextIndex"),
            QDBusConnection(QStringLiteral("test")),
            controller));

    QString capturedMethod;
    stub.set_lamda(static_cast<QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QList<QVariant> &)>(&QDBusAbstractInterface::asyncCallWithArgumentList),
                   [&](QDBusAbstractInterface *, const QString &method, const QList<QVariant> &) {
        __DBG_STUB_INVOKE__
        capturedMethod = method;
        return QDBusPendingCall::fromCompletedCall(makeReply());
    });

    controller->startIndexTask(true);

    EXPECT_EQ(capturedMethod, QStringLiteral("CreateIndexTask"));
}

TEST_F(AbstractIndexControllerImpl, StartIndexTask_Update_CallsUpdateIndexTask)
{
    controller = new AbstractIndexController(buildTestDescriptor([&]() {
        return QStringList { QStringLiteral("/home/test") };
    }));
    controller->interface.reset(new OrgDeepinFilemanagerTextIndexInterface(
            QStringLiteral("org.deepin.Filemanager.TextIndex"),
            QStringLiteral("/org/deepin/Filemanager/TextIndex"),
            QDBusConnection(QStringLiteral("test")),
            controller));

    QString capturedMethod;
    stub.set_lamda(static_cast<QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QList<QVariant> &)>(&QDBusAbstractInterface::asyncCallWithArgumentList),
                   [&](QDBusAbstractInterface *, const QString &method, const QList<QVariant> &) {
        __DBG_STUB_INVOKE__
        capturedMethod = method;
        return QDBusPendingCall::fromCompletedCall(makeReply());
    });

    controller->startIndexTask(false);

    EXPECT_EQ(capturedMethod, QStringLiteral("UpdateIndexTask"));
}

// ---------------------------------------------------------------------------
// handleConfigChanged
// ---------------------------------------------------------------------------

TEST_F(AbstractIndexControllerImpl, HandleConfigChanged_NonMatchingConfig_DoesNothing)
{
    controller = new AbstractIndexController(buildTestDescriptor());
    controller->isConfigEnabled = false;

    bool activeBackendCalled = false;
    stub.set_lamda(&AbstractIndexController::activeBackend, [&](AbstractIndexController *, bool) {
        __DBG_STUB_INVOKE__
        activeBackendCalled = true;
    });

    controller->handleConfigChanged(QStringLiteral("other.config"), QStringLiteral("enableFullTextSearch"));

    EXPECT_FALSE(activeBackendCalled);
    EXPECT_FALSE(controller->isConfigEnabled);
}

TEST_F(AbstractIndexControllerImpl, HandleConfigChanged_NonMatchingKey_DoesNothing)
{
    controller = new AbstractIndexController(buildTestDescriptor());
    controller->isConfigEnabled = false;

    bool activeBackendCalled = false;
    stub.set_lamda(&AbstractIndexController::activeBackend, [&](AbstractIndexController *, bool) {
        __DBG_STUB_INVOKE__
        activeBackendCalled = true;
    });

    controller->handleConfigChanged(QStringLiteral("org.deepin.dde.file-manager.search"), QStringLiteral("otherKey"));

    EXPECT_FALSE(activeBackendCalled);
    EXPECT_FALSE(controller->isConfigEnabled);
}

TEST_F(AbstractIndexControllerImpl, HandleConfigChanged_MatchingKey_UpdatesStateAndActivates)
{
    controller = new AbstractIndexController(buildTestDescriptor());
    controller->currentState = AbstractIndexController::State::Disabled;
    controller->isConfigEnabled = false;

    bool activeBackendCalled = false;
    bool updateTimerCalled = false;

    stub.set_lamda(&DConfigManager::instance, []() {
        __DBG_STUB_INVOKE__
        static DConfigManager manager;
        return &manager;
    });

    stub.set_lamda(&DConfigManager::value, [&](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });

    stub.set_lamda(&AbstractIndexController::activeBackend, [&](AbstractIndexController *, bool) {
        __DBG_STUB_INVOKE__
        activeBackendCalled = true;
    });

    stub.set_lamda(&AbstractIndexController::updateKeepAliveTimer, [&](AbstractIndexController *) {
        __DBG_STUB_INVOKE__
        updateTimerCalled = true;
    });

    controller->handleConfigChanged(QStringLiteral("org.deepin.dde.file-manager.search"), QStringLiteral("enableFullTextSearch"));

    EXPECT_TRUE(activeBackendCalled);
    EXPECT_TRUE(updateTimerCalled);
    EXPECT_TRUE(controller->isConfigEnabled);
}

// ---------------------------------------------------------------------------
// updateKeepAliveTimer
// ---------------------------------------------------------------------------

TEST_F(AbstractIndexControllerImpl, UpdateKeepAliveTimer_EnabledNotActive_StartsTimer)
{
    controller = new AbstractIndexController(buildTestDescriptor());
    controller->isConfigEnabled = true;

    bool startCalled = false;
    stub.set_lamda(static_cast<void (QTimer::*)()>(&QTimer::start), [&](QTimer *) {
        __DBG_STUB_INVOKE__
        startCalled = true;
    });

    stub.set_lamda(&QTimer::isActive, [](QTimer *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    controller->updateKeepAliveTimer();

    EXPECT_TRUE(startCalled);
}

TEST_F(AbstractIndexControllerImpl, UpdateKeepAliveTimer_DisabledActive_StopsTimer)
{
    controller = new AbstractIndexController(buildTestDescriptor());
    controller->isConfigEnabled = false;

    bool stopCalled = false;
    stub.set_lamda(&QTimer::stop, [&](QTimer *) {
        __DBG_STUB_INVOKE__
        stopCalled = true;
    });

    stub.set_lamda(&QTimer::isActive, [](QTimer *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    controller->updateKeepAliveTimer();

    EXPECT_TRUE(stopCalled);
}

// ---------------------------------------------------------------------------
// isTrackedPath
// ---------------------------------------------------------------------------

TEST_F(AbstractIndexControllerImpl, IsTrackedPath_EmptyProvider_MatchesHomePath)
{
    controller = new AbstractIndexController(buildDescriptorWithoutFactory());

    EXPECT_TRUE(controller->isTrackedPath(QDir::homePath()));
    EXPECT_FALSE(controller->isTrackedPath(QStringLiteral("/some/other/path")));
}

TEST_F(AbstractIndexControllerImpl, IsTrackedPath_WithProvider_MatchesListedPaths)
{
    QStringList paths { QStringLiteral("/path/one"), QStringLiteral("/path/two") };
    controller = new AbstractIndexController(buildTestDescriptor([&]() { return paths; }));

    EXPECT_TRUE(controller->isTrackedPath(QStringLiteral("/path/one")));
    EXPECT_TRUE(controller->isTrackedPath(QStringLiteral("/path/two")));
    EXPECT_FALSE(controller->isTrackedPath(QStringLiteral("/path/three")));
}
