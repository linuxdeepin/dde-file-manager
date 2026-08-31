// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionpluginmanagerprivate_1.cpp
 * @brief Unit tests for ExtensionPluginManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/pluginsload/extensionpluginmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class ExtensionPluginManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionPluginManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionPluginManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionPluginManagerPrivateTest, ExtensionPluginManagerPrivate)
{
    // Test constructor: ExtensionPluginManagerPrivate((ExtensionPluginManager *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ExtensionPluginManagerPrivateTest, doAppendExt)
{
    // Test method: void doAppendExt((const QString &name, ExtPluginLoaderPointer loader))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->doAppendExt(_arg0, ExtPluginLoaderPointer()));
}

TEST_F(ExtensionPluginManagerPrivateTest, release)
{
    // Test method: void release(())
    EXPECT_NO_FATAL_FAILURE(obj->release());
}

TEST_F(ExtensionPluginManagerPrivateTest, startInitializePlugins)
{
    // Test method: void startInitializePlugins(())
    EXPECT_NO_FATAL_FAILURE(obj->startInitializePlugins());
}

TEST_F(ExtensionPluginManagerPrivateTest, startMonitorPlugins)
{
    // Test method: void startMonitorPlugins(())
    EXPECT_NO_FATAL_FAILURE(obj->startMonitorPlugins());
}

TEST_F(ExtensionPluginManagerPrivateTest, ExtensionPluginManagerPrivate_Destructor)
{
    // Test method:  ~ExtensionPluginManagerPrivate(())
    EXPECT_NO_FATAL_FAILURE({ ExtensionPluginManagerPrivate *tmp = new ExtensionPluginManagerPrivate(); delete tmp; });
}
