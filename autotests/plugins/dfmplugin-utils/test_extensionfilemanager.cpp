// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/fileimpl/extensionfilemanager.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/pluginsload/extensionpluginmanager.h"

#include <dfm-base/utils/applaunchutils.h>
#include <dfm-extension/file/dfmextfileplugin.h>

#include <QTimer>
#include <QSignalSpy>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

class UT_ExtensionFileManager : public testing::Test
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

TEST_F(UT_ExtensionFileManager, instance_ReturnsSingleton)
{
    ExtensionFileManager &instance1 = ExtensionFileManager::instance();
    ExtensionFileManager &instance2 = ExtensionFileManager::instance();

    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(UT_ExtensionFileManager, initialize_PluginsInitialized_CallsOnAllPluginsInitialized)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, initialized),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(ADDR(ExtensionPluginManager, filePlugins),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QList<DFMEXT::DFMExtFilePlugin *>();
                   });

    ExtensionFileManager::instance().initialize();
}

TEST_F(UT_ExtensionFileManager, initialize_PluginsNotInitialized_EmitsRequest)
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

    ExtensionFileManager::instance().initialize();

    EXPECT_TRUE(timerCalled);
}

TEST_F(UT_ExtensionFileManager, onAllPluginsInitialized_EmptyPlugins_DoesNotAddStrategy)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, filePlugins),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QList<DFMEXT::DFMExtFilePlugin *>();
                   });

    ExtensionFileManager::instance().onAllPluginsInitialized();
}

TEST_F(UT_ExtensionFileManager, launch_EmptyContainer_ReturnsFalse)
{
    bool result = ExtensionFileManager::instance().launch("test.desktop", QStringList() << "/tmp/test.txt");

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionFileManager, launch_MultipleFiles_ReturnsFalse)
{
    QStringList files;
    files << "/tmp/file1.txt" << "/tmp/file2.txt" << "/tmp/file3.txt";

    bool result = ExtensionFileManager::instance().launch("test.desktop", files);

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionFileManager, launch_EmptyFileList_ReturnsFalse)
{
    bool result = ExtensionFileManager::instance().launch("test.desktop", QStringList());

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionFileManager, initialize_ConnectsSignal)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, initialized),
                   [] {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool singleShotCalled = false;
    typedef void (*SingleShotFunc)(int, const QObject *, const char *);
    stub.set_lamda(static_cast<SingleShotFunc>(QTimer::singleShot),
                   [&singleShotCalled](int, const QObject *, const char *) {
                       __DBG_STUB_INVOKE__
                       singleShotCalled = true;
                   });

    ExtensionFileManager::instance().initialize();

    EXPECT_TRUE(singleShotCalled);
}

TEST_F(UT_ExtensionFileManager, launch_WithDesktopFile_ReturnsFalse)
{
    bool result = ExtensionFileManager::instance().launch("/usr/share/applications/deepin-editor.desktop",
                                                          QStringList() << "/tmp/test.txt");

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionFileManager, launch_WithSpecialChars_ReturnsFalse)
{
    QStringList files;
    files << "/tmp/文件.txt" << "/tmp/file with spaces.txt";

    bool result = ExtensionFileManager::instance().launch("test.desktop", files);

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionFileManager, onAllPluginsInitialized_CalledMultipleTimes_OnlyInitializesOnce)
{
    stub.set_lamda(ADDR(ExtensionPluginManager, filePlugins),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QList<DFMEXT::DFMExtFilePlugin *>();
                   });

    ExtensionFileManager::instance().onAllPluginsInitialized();
    ExtensionFileManager::instance().onAllPluginsInitialized();
}

TEST_F(UT_ExtensionFileManager, launch_VariousDesktopFiles_ReturnsFalse)
{
    QStringList files = { "/home/user/document.pdf" };

    bool result1 = ExtensionFileManager::instance().launch("org.gnome.Evince.desktop", files);
    bool result2 = ExtensionFileManager::instance().launch("libreoffice-writer.desktop", files);

    EXPECT_FALSE(result1);
    EXPECT_FALSE(result2);
}

TEST_F(UT_ExtensionFileManager, initialize_WhenPluginsInitialized_CallsOnAllPluginsInitialized)
{
    bool onAllCalled = false;

    stub.set_lamda(ADDR(ExtensionPluginManager, initialized),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(ADDR(ExtensionPluginManager, filePlugins),
                   [&onAllCalled] {
                       __DBG_STUB_INVOKE__
                       onAllCalled = true;
                       return QList<DFMEXT::DFMExtFilePlugin *>();
                   });

    ExtensionFileManager::instance().initialize();

    EXPECT_TRUE(onAllCalled);
}

TEST_F(UT_ExtensionFileManager, launch_WithAbsolutePaths_ReturnsFalse)
{
    QStringList files = {
        "/home/user/Documents/test.txt",
        "/var/log/syslog",
        "/etc/hosts"
    };

    bool result = ExtensionFileManager::instance().launch("gedit.desktop", files);

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionFileManager, launch_WithSingleFile_ReturnsFalse)
{
    bool result = ExtensionFileManager::instance().launch("vim.desktop",
                                                          QStringList() << "/tmp/single.txt");

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionFileManager, launch_WithLargeFileList_ReturnsFalse)
{
    QStringList files;
    for (int i = 0; i < 100; ++i) {
        files << QString("/tmp/file%1.txt").arg(i);
    }

    bool result = ExtensionFileManager::instance().launch("test.desktop", files);

    EXPECT_FALSE(result);
}

