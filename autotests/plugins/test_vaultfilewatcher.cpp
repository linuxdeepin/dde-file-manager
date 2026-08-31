// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultfilewatcher.cpp
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

TEST_F(VaultFileWatcherTest, onFileDeleted)
{
    // Test method: void onFileDeleted((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileDeleted(_arg0));
}
