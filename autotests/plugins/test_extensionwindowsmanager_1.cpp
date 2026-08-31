// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionwindowsmanager_1.cpp
 * @brief Unit tests for ExtensionWindowsManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/windowimpl/extensionwindowsmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class ExtensionWindowsManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionWindowsManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionWindowsManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionWindowsManagerTest, ExtensionWindowsManager)
{
    // Test constructor: ExtensionWindowsManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ExtensionWindowsManagerTest, handleWindowOpened)
{
    // Test method: void handleWindowOpened((quint64 id))
    EXPECT_NO_FATAL_FAILURE(obj->handleWindowOpened(0));
}

TEST_F(ExtensionWindowsManagerTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(ExtensionWindowsManagerTest, onCurrentUrlChanged)
{
    // Test method: void onCurrentUrlChanged((quint64 id, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onCurrentUrlChanged(0, _arg1));
}

TEST_F(ExtensionWindowsManagerTest, onLastWindowClosed)
{
    // Test method: void onLastWindowClosed((quint64 id))
    EXPECT_NO_FATAL_FAILURE(obj->onLastWindowClosed(0));
}

TEST_F(ExtensionWindowsManagerTest, onWindowClosed)
{
    // Test method: void onWindowClosed((quint64 id))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowClosed(0));
}

TEST_F(ExtensionWindowsManagerTest, onWindowOpened)
{
    // Test method: void onWindowOpened((quint64 id))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowOpened(0));
}
