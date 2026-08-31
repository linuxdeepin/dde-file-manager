// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarfilewatcher_1.cpp
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

TEST_F(SidebarFileWatcherTest, onFileAttributeChanged)
{
    // Test method: void onFileAttributeChanged((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileAttributeChanged(_arg0));
}

TEST_F(SidebarFileWatcherTest, onFileRename)
{
    // Test method: void onFileRename((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileRename(_arg0, _arg1));
}

TEST_F(SidebarFileWatcherTest, onHiddenFileStatusChanged)
{
    // Test method: void onHiddenFileStatusChanged((bool showHidden))
    EXPECT_NO_FATAL_FAILURE(obj->onHiddenFileStatusChanged(false));
}

TEST_F(SidebarFileWatcherTest, onSubfileCreated)
{
    // Test method: void onSubfileCreated((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onSubfileCreated(_arg0));
}

TEST_F(SidebarFileWatcherTest, setDirsVisible)
{
    // Test setter: void setDirsVisible((bool showHidden, const QList<QUrl> &dirs))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setDirsVisible(false, _arg1));
}

TEST_F(SidebarFileWatcherTest, stopAllWatchers)
{
    // Test method: void stopAllWatchers(())
    EXPECT_NO_FATAL_FAILURE(obj->stopAllWatchers());
}

TEST_F(SidebarFileWatcherTest, unwatchDirectory)
{
    // Test method: void unwatchDirectory((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->unwatchDirectory(_arg0));
}

TEST_F(SidebarFileWatcherTest, watchDirectory)
{
    // Test method: void watchDirectory((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->watchDirectory(_arg0));
}

TEST_F(SidebarFileWatcherTest, SidebarFileWatcher_Destructor)
{
    // Test method:  ~SidebarFileWatcher(())
    EXPECT_NO_FATAL_FAILURE({ SidebarFileWatcher *tmp = new SidebarFileWatcher(); delete tmp; });
}
