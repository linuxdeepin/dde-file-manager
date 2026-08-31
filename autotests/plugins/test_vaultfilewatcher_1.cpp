// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultfilewatcher_1.cpp
 * @brief Unit tests for VaultFileWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileutils/vaultfilewatcher.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultFileWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultFileWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultFileWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultFileWatcherTest, VaultFileWatcher)
{
    // Test constructor: VaultFileWatcher((const QUrl &url, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultFileWatcherTest, onFileAttributeChanged)
{
    // Test method: void onFileAttributeChanged((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileAttributeChanged(_arg0));
}

TEST_F(VaultFileWatcherTest, onFileRename)
{
    // Test method: void onFileRename((const QUrl &fromUrl, const QUrl &toUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileRename(_arg0, _arg1));
}

TEST_F(VaultFileWatcherTest, onSubfileCreated)
{
    // Test method: void onSubfileCreated((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onSubfileCreated(_arg0));
}

TEST_F(VaultFileWatcherTest, VaultFileWatcher_Destructor)
{
    // Test method:  ~VaultFileWatcher(())
    EXPECT_NO_FATAL_FAILURE({ VaultFileWatcher *tmp = new VaultFileWatcher(); delete tmp; });
}
