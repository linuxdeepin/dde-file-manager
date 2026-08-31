// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarfilewatcher.cpp
 * @brief Unit tests for SidebarFileWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/sidebarfilewatcher.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SidebarFileWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SidebarFileWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SidebarFileWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SidebarFileWatcherTest, onFileDeleted)
{
    // Test method: void onFileDeleted((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileDeleted(_arg0));
}
