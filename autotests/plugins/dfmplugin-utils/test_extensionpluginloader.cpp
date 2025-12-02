// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/pluginsload/extensionpluginloader.h"

#include <QLibrary>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

class UT_ExtensionPluginLoader : public testing::Test
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

TEST_F(UT_ExtensionPluginLoader, Constructor_SetsFileName)
{
    ExtensionPluginLoader loader("/path/to/plugin.so");

    EXPECT_EQ(loader.fileName(), "/path/to/plugin.so");
}

TEST_F(UT_ExtensionPluginLoader, fileName_ReturnsLoaderFileName)
{
    ExtensionPluginLoader loader("/test/plugin.so");

    QString result = loader.fileName();

    EXPECT_EQ(result, "/test/plugin.so");
}

TEST_F(UT_ExtensionPluginLoader, lastError_InitiallyEmpty)
{
    ExtensionPluginLoader loader("/test/plugin.so");

    QString result = loader.lastError();

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_ExtensionPluginLoader, loadPlugin_EmptyFileName_ReturnsFalse)
{
    ExtensionPluginLoader loader("");

    bool result = loader.loadPlugin();

    EXPECT_FALSE(result);
    EXPECT_FALSE(loader.lastError().isEmpty());
}

TEST_F(UT_ExtensionPluginLoader, loadPlugin_LoadFailed_ReturnsFalse)
{
    stub.set_lamda(ADDR(QLibrary, load),
                   [](QLibrary *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(ADDR(QLibrary, errorString),
                   [](QLibrary *) -> QString {
                       __DBG_STUB_INVOKE__
                       return "Load error";
                   });

    ExtensionPluginLoader loader("/nonexistent/plugin.so");

    bool result = loader.loadPlugin();

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionPluginLoader, loadPlugin_LoadSuccess_ReturnsTrue)
{
    stub.set_lamda(ADDR(QLibrary, load),
                   [](QLibrary *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    ExtensionPluginLoader loader("/test/plugin.so");

    bool result = loader.loadPlugin();

    EXPECT_TRUE(result);
}

TEST_F(UT_ExtensionPluginLoader, initialize_NotLoaded_ReturnsFalse)
{
    stub.set_lamda(ADDR(QLibrary, isLoaded),
                   [](QLibrary *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    ExtensionPluginLoader loader("/test/plugin.so");

    bool result = loader.initialize();

    EXPECT_FALSE(result);
    EXPECT_FALSE(loader.lastError().isEmpty());
}

TEST_F(UT_ExtensionPluginLoader, initialize_ResolveFailed_ReturnsFalse)
{
    stub.set_lamda(ADDR(QLibrary, isLoaded),
                   [](QLibrary *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(static_cast<QFunctionPointer (QLibrary::*)(const char *)>(&QLibrary::resolve),
                   [](QLibrary *, const char *) -> QFunctionPointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    ExtensionPluginLoader loader("/test/plugin.so");

    bool result = loader.initialize();

    EXPECT_FALSE(result);
}

TEST_F(UT_ExtensionPluginLoader, resolveMenuPlugin_NotLoaded_ReturnsNull)
{
    stub.set_lamda(ADDR(QLibrary, isLoaded),
                   [](QLibrary *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    ExtensionPluginLoader loader("/test/plugin.so");

    DFMEXT::DFMExtMenuPlugin *result = loader.resolveMenuPlugin();

    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_ExtensionPluginLoader, resolveEmblemPlugin_NotLoaded_ReturnsNull)
{
    stub.set_lamda(ADDR(QLibrary, isLoaded),
                   [](QLibrary *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    ExtensionPluginLoader loader("/test/plugin.so");

    DFMEXT::DFMExtEmblemIconPlugin *result = loader.resolveEmblemPlugin();

    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_ExtensionPluginLoader, resolveWindowPlugin_NotLoaded_ReturnsNull)
{
    stub.set_lamda(ADDR(QLibrary, isLoaded),
                   [](QLibrary *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    ExtensionPluginLoader loader("/test/plugin.so");

    DFMEXT::DFMExtWindowPlugin *result = loader.resolveWindowPlugin();

    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_ExtensionPluginLoader, resolveFilePlugin_NotLoaded_ReturnsNull)
{
    stub.set_lamda(ADDR(QLibrary, isLoaded),
                   [](QLibrary *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    ExtensionPluginLoader loader("/test/plugin.so");

    DFMEXT::DFMExtFilePlugin *result = loader.resolveFilePlugin();

    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_ExtensionPluginLoader, shutdown_ResolveFailed_ReturnsFalse)
{
    stub.set_lamda(static_cast<QFunctionPointer (QLibrary::*)(const char *)>(&QLibrary::resolve),
                   [](QLibrary *, const char *) -> QFunctionPointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    ExtensionPluginLoader loader("/test/plugin.so");

    bool result = loader.shutdown();

    EXPECT_FALSE(result);
}

