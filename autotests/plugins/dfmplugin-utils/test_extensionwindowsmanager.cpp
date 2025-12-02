// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/windowimpl/extensionwindowsmanager.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/pluginsload/extensionpluginmanager.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-extension/window/dfmextwindowplugin.h>

#include <QUrl>
#include <QTimer>
#include <QSignalSpy>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

class UT_ExtensionWindowsManager : public testing::Test
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

TEST_F(UT_ExtensionWindowsManager, instance_ReturnsSingleton)
{
    ExtensionWindowsManager &instance1 = ExtensionWindowsManager::instance();
    ExtensionWindowsManager &instance2 = ExtensionWindowsManager::instance();

    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(UT_ExtensionWindowsManager, initialize_ConnectsSignals)
{
    ExtensionWindowsManager::instance().initialize();
}

TEST_F(UT_ExtensionWindowsManager, onWindowOpened_PluginsNotInitialized_EmitsRequestInit)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, initialized),
                   [] {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool timerCalled = false;
    stub.set_lamda((void (*)(int, const QObject *, const char *))QTimer::singleShot,
                   [&timerCalled](int, const QObject *, const char *) {
                       __DBG_STUB_INVOKE__
                       timerCalled = true;
                   });

    ExtensionWindowsManager::instance().onWindowOpened(1);

    EXPECT_TRUE(timerCalled);
}

TEST_F(UT_ExtensionWindowsManager, onWindowOpened_PluginsInitialized_CallsHandleWindow)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, initialized),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(ADDR(ExtensionPluginManager, windowPlugins),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QList<DFMEXT::DFMExtWindowPlugin *>();
                   });

    ExtensionWindowsManager::instance().onWindowOpened(100);
}

TEST_F(UT_ExtensionWindowsManager, onWindowClosed_PluginsNotInitialized_ReturnsEarly)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, initialized),
                   [] {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    ExtensionWindowsManager::instance().onWindowClosed(1);
}

TEST_F(UT_ExtensionWindowsManager, onWindowClosed_PluginsInitialized_CallsPlugins)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, initialized),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(ADDR(ExtensionPluginManager, windowPlugins),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QList<DFMEXT::DFMExtWindowPlugin *>();
                   });

    ExtensionWindowsManager::instance().onWindowClosed(1);
}

TEST_F(UT_ExtensionWindowsManager, onLastWindowClosed_PluginsNotInitialized_ReturnsEarly)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, initialized),
                   [] {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    ExtensionWindowsManager::instance().onLastWindowClosed(1);
}

TEST_F(UT_ExtensionWindowsManager, onLastWindowClosed_PluginsInitialized_CallsPlugins)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, initialized),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(ADDR(ExtensionPluginManager, windowPlugins),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QList<DFMEXT::DFMExtWindowPlugin *>();
                   });

    ExtensionWindowsManager::instance().onLastWindowClosed(1);
}

TEST_F(UT_ExtensionWindowsManager, onCurrentUrlChanged_PluginsNotInitialized_ReturnsEarly)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, initialized),
                   [] {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    ExtensionWindowsManager::instance().onCurrentUrlChanged(1, QUrl::fromLocalFile("/tmp"));
}

TEST_F(UT_ExtensionWindowsManager, onCurrentUrlChanged_PluginsInitialized_CallsPlugins)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, initialized),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(ADDR(ExtensionPluginManager, windowPlugins),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QList<DFMEXT::DFMExtWindowPlugin *>();
                   });

    ExtensionWindowsManager::instance().onCurrentUrlChanged(1, QUrl::fromLocalFile("/home"));
}

TEST_F(UT_ExtensionWindowsManager, onAllPluginsInitialized_FirstWinIdZero_DoesNothing)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, initialized),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(ADDR(ExtensionPluginManager, windowPlugins),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QList<DFMEXT::DFMExtWindowPlugin *>();
                   });

    ExtensionWindowsManager::instance().onAllPluginsInitialized();
}

