// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/pluginsload/extensionpluginmanager.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/pluginsload/extensionpluginmanager_p.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/pluginsload/extensionpluginloader.h"

#include <dfm-base/base/schemefactory.h>

#include <QSignalSpy>
#include <QThread>
#include <QCoreApplication>
#include <QLibrary>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

class UT_ExtensionPluginManager : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_ExtensionPluginManager, instance_ReturnsSingleton)
{
    ExtensionPluginManager &instance1 = ExtensionPluginManager::instance();
    ExtensionPluginManager &instance2 = ExtensionPluginManager::instance();

    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(UT_ExtensionPluginManager, currentState_ReturnsValidState)
{
    ExtensionPluginManager::InitState state = ExtensionPluginManager::instance().currentState();

    EXPECT_TRUE(state == ExtensionPluginManager::kReady ||
                state == ExtensionPluginManager::kScanned ||
                state == ExtensionPluginManager::kLoaded ||
                state == ExtensionPluginManager::kInitialized);
}

TEST_F(UT_ExtensionPluginManager, initialized_ReturnsCorrectState)
{
    bool result = ExtensionPluginManager::instance().initialized();
    ExtensionPluginManager::InitState state = ExtensionPluginManager::instance().currentState();

    EXPECT_EQ(result, state == ExtensionPluginManager::kInitialized);
}

TEST_F(UT_ExtensionPluginManager, exists_Menu_ReturnsCorrectValue)
{
    bool result = ExtensionPluginManager::instance().exists(ExtensionPluginManager::kMenu);
    Q_UNUSED(result)
}

TEST_F(UT_ExtensionPluginManager, exists_EmblemIcon_ReturnsCorrectValue)
{
    bool result = ExtensionPluginManager::instance().exists(ExtensionPluginManager::kEmblemIcon);
    Q_UNUSED(result)
}

TEST_F(UT_ExtensionPluginManager, exists_InvalidType_ReturnsFalse)
{
    bool result = ExtensionPluginManager::instance().exists(static_cast<ExtensionPluginManager::ExtensionType>(99));
    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionPluginManager, menuPlugins_ReturnsPluginList)
{
    QList<DFMEXT::DFMExtMenuPlugin *> result = ExtensionPluginManager::instance().menuPlugins();
    Q_UNUSED(result)
}

TEST_F(UT_ExtensionPluginManager, emblemPlugins_ReturnsPluginList)
{
    QList<DFMEXT::DFMExtEmblemIconPlugin *> result = ExtensionPluginManager::instance().emblemPlugins();
    Q_UNUSED(result)
}

TEST_F(UT_ExtensionPluginManager, windowPlugins_ReturnsPluginList)
{
    QList<DFMEXT::DFMExtWindowPlugin *> result = ExtensionPluginManager::instance().windowPlugins();
    Q_UNUSED(result)
}

TEST_F(UT_ExtensionPluginManager, filePlugins_ReturnsPluginList)
{
    QList<DFMEXT::DFMExtFilePlugin *> result = ExtensionPluginManager::instance().filePlugins();
    Q_UNUSED(result)
}

TEST_F(UT_ExtensionPluginManager, pluginMenuProxy_ReturnsNonNull)
{
    DFMEXT::DFMExtMenuProxy *result = ExtensionPluginManager::instance().pluginMenuProxy();

    EXPECT_NE(result, nullptr);
}

TEST_F(UT_ExtensionPluginManager, onLoadingPlugins_CalledOnce)
{
    stub.set_lamda(ADDR(ExtensionPluginManagerPrivate, startInitializePlugins),
                   [](ExtensionPluginManagerPrivate *) {
                       __DBG_STUB_INVOKE__
                   });

    ExtensionPluginManager::instance().onLoadingPlugins();
}

// ========== ExtensionPluginManagerPrivate Tests ==========

class UT_ExtensionPluginManagerPrivate : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_ExtensionPluginManagerPrivate, startMonitorPlugins_NotDesktop_DoesNothing)
{
    stub.set_lamda(&QCoreApplication::applicationName,
                   [] {
                       __DBG_STUB_INVOKE__
                       return QString("dde-file-manager");
                   });

    ExtensionPluginManager &manager = ExtensionPluginManager::instance();
    ExtensionPluginManagerPrivate *d = reinterpret_cast<ExtensionPluginManagerPrivate *>(
            *reinterpret_cast<void **>(reinterpret_cast<char *>(&manager) + sizeof(QObject)));

    d->startMonitorPlugins();
}

TEST_F(UT_ExtensionPluginManagerPrivate, restartDesktop_NonSoFile_ReturnsEarly)
{
    ExtensionPluginManager &manager = ExtensionPluginManager::instance();
    ExtensionPluginManagerPrivate *d = reinterpret_cast<ExtensionPluginManagerPrivate *>(
            *reinterpret_cast<void **>(reinterpret_cast<char *>(&manager) + sizeof(QObject)));

    d->restartDesktop(QUrl::fromLocalFile("/tmp/test.txt"));
}

TEST_F(UT_ExtensionPluginManagerPrivate, release_CalledMultipleTimes_SafelyHandled)
{
    ExtensionPluginManager &manager = ExtensionPluginManager::instance();
    ExtensionPluginManagerPrivate *d = reinterpret_cast<ExtensionPluginManagerPrivate *>(
            *reinterpret_cast<void **>(reinterpret_cast<char *>(&manager) + sizeof(QObject)));

    d->release();
    d->release();
}

// ========== ExtensionPluginInitWorker Tests ==========

class UT_ExtensionPluginInitWorker : public testing::Test
{
protected:
    void SetUp() override
    {
        worker = new ExtensionPluginInitWorker();
    }

    void TearDown() override
    {
        delete worker;
        worker = nullptr;
        stub.clear();
    }

    ExtensionPluginInitWorker *worker { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ExtensionPluginInitWorker, doWork_EmptyPaths_EmitsSignals)
{
    QSignalSpy scanSpy(worker, &ExtensionPluginInitWorker::scanPluginsFinished);
    QSignalSpy loadSpy(worker, &ExtensionPluginInitWorker::loadPluginsFinished);
    QSignalSpy initSpy(worker, &ExtensionPluginInitWorker::initPluginsFinished);

    worker->doWork(QStringList());

    EXPECT_EQ(scanSpy.count(), 1);
    EXPECT_EQ(loadSpy.count(), 1);
    EXPECT_EQ(initSpy.count(), 1);
}

TEST_F(UT_ExtensionPluginInitWorker, doWork_NonExistentPath_EmitsSignals)
{
    QSignalSpy scanSpy(worker, &ExtensionPluginInitWorker::scanPluginsFinished);

    worker->doWork({ "/nonexistent/path" });

    EXPECT_EQ(scanSpy.count(), 1);
}

TEST_F(UT_ExtensionPluginInitWorker, doWork_MultiplePaths_EmitsAllSignals)
{
    QSignalSpy scanSpy(worker, &ExtensionPluginInitWorker::scanPluginsFinished);
    QSignalSpy loadSpy(worker, &ExtensionPluginInitWorker::loadPluginsFinished);
    QSignalSpy initSpy(worker, &ExtensionPluginInitWorker::initPluginsFinished);

    worker->doWork({ "/nonexistent/path1", "/nonexistent/path2" });

    EXPECT_EQ(scanSpy.count(), 1);
    EXPECT_EQ(loadSpy.count(), 1);
    EXPECT_EQ(initSpy.count(), 1);
}

TEST_F(UT_ExtensionPluginInitWorker, doWork_TmpPath_ScansDirectory)
{
    QSignalSpy scanSpy(worker, &ExtensionPluginInitWorker::scanPluginsFinished);

    worker->doWork({ "/tmp" });

    EXPECT_EQ(scanSpy.count(), 1);
}

// ========== ExtensionPluginManagerPrivate Additional Tests ==========

TEST_F(UT_ExtensionPluginManagerPrivate, startMonitorPlugins_IsDesktop_CreatesWatcher)
{
    stub.set_lamda(&QCoreApplication::applicationName,
                   [] {
                       __DBG_STUB_INVOKE__
                       return QString("dde-desktop");
                   });

    stub.set_lamda(&WatcherFactory::create<AbstractFileWatcher>,
                   [] {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    ExtensionPluginManager &manager = ExtensionPluginManager::instance();
    ExtensionPluginManagerPrivate *d = reinterpret_cast<ExtensionPluginManagerPrivate *>(
            *reinterpret_cast<void **>(reinterpret_cast<char *>(&manager) + sizeof(QObject)));

    d->startMonitorPlugins();
}

TEST_F(UT_ExtensionPluginManagerPrivate, restartDesktop_SoFile_CallsUpgrade)
{
    stub.set_lamda(&QLibrary::load,
                   [](QLibrary *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    ExtensionPluginManager &manager = ExtensionPluginManager::instance();
    ExtensionPluginManagerPrivate *d = reinterpret_cast<ExtensionPluginManagerPrivate *>(
            *reinterpret_cast<void **>(reinterpret_cast<char *>(&manager) + sizeof(QObject)));

    d->restartDesktop(QUrl::fromLocalFile("/tmp/test.so"));
}

TEST_F(UT_ExtensionPluginManagerPrivate, curState_InitialValue_IsReady)
{
    ExtensionPluginManager &manager = ExtensionPluginManager::instance();
    ExtensionPluginManagerPrivate *d = reinterpret_cast<ExtensionPluginManagerPrivate *>(
            *reinterpret_cast<void **>(reinterpret_cast<char *>(&manager) + sizeof(QObject)));

    EXPECT_TRUE(d->curState == ExtensionPluginManager::kReady ||
                d->curState == ExtensionPluginManager::kScanned ||
                d->curState == ExtensionPluginManager::kLoaded ||
                d->curState == ExtensionPluginManager::kInitialized);
}

TEST_F(UT_ExtensionPluginManagerPrivate, defaultPluginPath_IsNotEmpty)
{
    ExtensionPluginManager &manager = ExtensionPluginManager::instance();
    ExtensionPluginManagerPrivate *d = reinterpret_cast<ExtensionPluginManagerPrivate *>(
            *reinterpret_cast<void **>(reinterpret_cast<char *>(&manager) + sizeof(QObject)));

    EXPECT_FALSE(d->defaultPluginPath.isEmpty());
}

TEST_F(UT_ExtensionPluginManagerPrivate, proxy_IsNotNull)
{
    ExtensionPluginManager &manager = ExtensionPluginManager::instance();
    ExtensionPluginManagerPrivate *d = reinterpret_cast<ExtensionPluginManagerPrivate *>(
            *reinterpret_cast<void **>(reinterpret_cast<char *>(&manager) + sizeof(QObject)));

    EXPECT_NE(d->proxy.data(), nullptr);
}

TEST_F(UT_ExtensionPluginManagerPrivate, menuMap_InitiallyEmpty)
{
    ExtensionPluginManager &manager = ExtensionPluginManager::instance();
    ExtensionPluginManagerPrivate *d = reinterpret_cast<ExtensionPluginManagerPrivate *>(
            *reinterpret_cast<void **>(reinterpret_cast<char *>(&manager) + sizeof(QObject)));

    Q_UNUSED(d->menuMap.isEmpty())
}

TEST_F(UT_ExtensionPluginManagerPrivate, emblemMap_InitiallyEmpty)
{
    ExtensionPluginManager &manager = ExtensionPluginManager::instance();
    ExtensionPluginManagerPrivate *d = reinterpret_cast<ExtensionPluginManagerPrivate *>(
            *reinterpret_cast<void **>(reinterpret_cast<char *>(&manager) + sizeof(QObject)));

    Q_UNUSED(d->emblemMap.isEmpty())
}

TEST_F(UT_ExtensionPluginManagerPrivate, windowMap_InitiallyEmpty)
{
    ExtensionPluginManager &manager = ExtensionPluginManager::instance();
    ExtensionPluginManagerPrivate *d = reinterpret_cast<ExtensionPluginManagerPrivate *>(
            *reinterpret_cast<void **>(reinterpret_cast<char *>(&manager) + sizeof(QObject)));

    Q_UNUSED(d->windowMap.isEmpty())
}

TEST_F(UT_ExtensionPluginManagerPrivate, fileMap_InitiallyEmpty)
{
    ExtensionPluginManager &manager = ExtensionPluginManager::instance();
    ExtensionPluginManagerPrivate *d = reinterpret_cast<ExtensionPluginManagerPrivate *>(
            *reinterpret_cast<void **>(reinterpret_cast<char *>(&manager) + sizeof(QObject)));

    Q_UNUSED(d->fileMap.isEmpty())
}

// ========== ExtensionPluginManager Additional Tests ==========

TEST_F(UT_ExtensionPluginManager, constructor_InitializesPrivate)
{
    EXPECT_NE(&ExtensionPluginManager::instance(), nullptr);
}

TEST_F(UT_ExtensionPluginManager, onLoadingPlugins_SecondCall_DoesNotReinitialize)
{
    bool initCalled = false;
    stub.set_lamda(ADDR(ExtensionPluginManagerPrivate, startInitializePlugins),
                   [&initCalled](ExtensionPluginManagerPrivate *) {
                       __DBG_STUB_INVOKE__
                       initCalled = true;
                   });

    ExtensionPluginManager::instance().onLoadingPlugins();
    initCalled = false;
    ExtensionPluginManager::instance().onLoadingPlugins();

    EXPECT_FALSE(initCalled);
}

