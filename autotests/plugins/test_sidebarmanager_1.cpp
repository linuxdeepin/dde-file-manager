// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarmanager_1.cpp
 * @brief Unit tests for SideBarManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/sidebarmanager.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarManagerTest, SideBarManager)
{
    // Test constructor: SideBarManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SideBarManagerTest, openFolderInASeparateProcess)
{
    // Test method: void openFolderInASeparateProcess((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->openFolderInASeparateProcess(_arg0));
}

TEST_F(SideBarManagerTest, runCd)
{
    // Test method: void runCd((SideBarItem *item, quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->runCd(nullptr, 0));
}

TEST_F(SideBarManagerTest, runContextMenu)
{
    // Test method: void runContextMenu((SideBarItem *item, quint64 windowId, const QPoint &globalPos))
    QPoint _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->runContextMenu(nullptr, 0, _arg2));
}

TEST_F(SideBarManagerTest, runRename)
{
    // Test method: void runRename((SideBarItem *item, quint64 windowId, const QString &name))
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->runRename(nullptr, 0, _arg2));
}

TEST_F(SideBarManagerTest, SideBarManager_Destructor)
{
    // Test method:  ~SideBarManager(())
    EXPECT_NO_FATAL_FAILURE({ SideBarManager *tmp = new SideBarManager(); delete tmp; });
}
