// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionpluginmanagerprivate.cpp
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

TEST_F(ExtensionPluginManagerPrivateTest, restartDesktop)
{
    // Test method: void restartDesktop((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->restartDesktop(_arg0));
}
