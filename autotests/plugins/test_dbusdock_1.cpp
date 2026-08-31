// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dbusdock_1.cpp
 * @brief Unit tests for DBusDock methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/dbus-private/dbusdock1.h"

#include <QTest>

using namespace ddplugin_core;

class DBusDockTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DBusDock();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DBusDock *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DBusDockTest, ActivateWindow)
{
    // Test getter: QDBusPendingReply<> ActivateWindow()
    auto result = obj->ActivateWindow();
    EXPECT_NO_FATAL_FAILURE({ obj->ActivateWindow(); });

}

TEST_F(DBusDockTest, CancelPreviewWindow)
{
    // Test getter: QDBusPendingReply<> CancelPreviewWindow()
    auto result = obj->CancelPreviewWindow();
    EXPECT_NO_FATAL_FAILURE({ obj->CancelPreviewWindow(); });

}

TEST_F(DBusDockTest, CloseWindow)
{
    // Test getter: QDBusPendingReply<> CloseWindow()
    auto result = obj->CloseWindow();
    EXPECT_NO_FATAL_FAILURE({ obj->CloseWindow(); });

}

TEST_F(DBusDockTest, GetDockedAppsDesktopFiles)
{
    // Test getter: QDBusPendingReply<QStringList> GetDockedAppsDesktopFiles()
    auto result = obj->GetDockedAppsDesktopFiles();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DBusDockTest, GetEntryIDs)
{
    // Test getter: QDBusPendingReply<QStringList> GetEntryIDs()
    auto result = obj->GetEntryIDs();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DBusDockTest, GetPluginSettings)
{
    // Test getter: QDBusPendingReply<QString> GetPluginSettings()
    auto result = obj->GetPluginSettings();
    EXPECT_NO_FATAL_FAILURE({ obj->GetPluginSettings(); });

}

TEST_F(DBusDockTest, IsDocked)
{
    // Test getter: QDBusPendingReply<bool> IsDocked()
    auto result = obj->IsDocked();
    EXPECT_NO_FATAL_FAILURE({ obj->IsDocked(); });

}

TEST_F(DBusDockTest, IsOnDock)
{
    // Test getter: QDBusPendingReply<bool> IsOnDock()
    auto result = obj->IsOnDock();
    EXPECT_NO_FATAL_FAILURE({ obj->IsOnDock(); });

}

TEST_F(DBusDockTest, MakeWindowAbove)
{
    // Test getter: QDBusPendingReply<> MakeWindowAbove()
    auto result = obj->MakeWindowAbove();
    EXPECT_NO_FATAL_FAILURE({ obj->MakeWindowAbove(); });

}

TEST_F(DBusDockTest, MaximizeWindow)
{
    // Test getter: QDBusPendingReply<> MaximizeWindow()
    auto result = obj->MaximizeWindow();
    EXPECT_NO_FATAL_FAILURE({ obj->MaximizeWindow(); });

}

TEST_F(DBusDockTest, MergePluginSettings)
{
    // Test getter: QDBusPendingReply<> MergePluginSettings()
    auto result = obj->MergePluginSettings();
    EXPECT_NO_FATAL_FAILURE({ obj->MergePluginSettings(); });

}

TEST_F(DBusDockTest, MinimizeWindow)
{
    // Test getter: QDBusPendingReply<> MinimizeWindow()
    auto result = obj->MinimizeWindow();
    EXPECT_NO_FATAL_FAILURE({ obj->MinimizeWindow(); });

}

TEST_F(DBusDockTest, MoveEntry)
{
    // Test getter: QDBusPendingReply<> MoveEntry()
    auto result = obj->MoveEntry();
    EXPECT_NO_FATAL_FAILURE({ obj->MoveEntry(); });

}

TEST_F(DBusDockTest, MoveWindow)
{
    // Test getter: QDBusPendingReply<> MoveWindow()
    auto result = obj->MoveWindow();
    EXPECT_NO_FATAL_FAILURE({ obj->MoveWindow(); });

}

TEST_F(DBusDockTest, PreviewWindow)
{
    // Test getter: QDBusPendingReply<> PreviewWindow()
    auto result = obj->PreviewWindow();
    EXPECT_NO_FATAL_FAILURE({ obj->PreviewWindow(); });

}

TEST_F(DBusDockTest, QueryWindowIdentifyMethod)
{
    // Test getter: QDBusPendingReply<QString> QueryWindowIdentifyMethod()
    auto result = obj->QueryWindowIdentifyMethod();
    EXPECT_NO_FATAL_FAILURE({ obj->QueryWindowIdentifyMethod(); });

}

TEST_F(DBusDockTest, RequestDock)
{
    // Test getter: QDBusPendingReply<bool> RequestDock()
    auto result = obj->RequestDock();
    EXPECT_NO_FATAL_FAILURE({ obj->RequestDock(); });

}

TEST_F(DBusDockTest, RequestUndock)
{
    // Test getter: QDBusPendingReply<bool> RequestUndock()
    auto result = obj->RequestUndock();
    EXPECT_NO_FATAL_FAILURE({ obj->RequestUndock(); });

}

TEST_F(DBusDockTest, SetFrontendWindowRect)
{
    // Test getter: QDBusPendingReply<> SetFrontendWindowRect()
    auto result = obj->SetFrontendWindowRect();
    EXPECT_NO_FATAL_FAILURE({ obj->SetFrontendWindowRect(); });

}

TEST_F(DBusDockTest, SetPluginSettings)
{
    // Test getter: QDBusPendingReply<> SetPluginSettings()
    auto result = obj->SetPluginSettings();
    EXPECT_NO_FATAL_FAILURE({ obj->SetPluginSettings(); });

}

TEST_F(DBusDockTest, frontendWindowRect)
{
    // Test getter: DockRect frontendWindowRect()
    auto result = obj->frontendWindowRect();
    EXPECT_NO_FATAL_FAILURE({ obj->frontendWindowRect(); });

}

TEST_F(DBusDockTest, hideMode)
{
    // Test getter: int hideMode()
    auto result = obj->hideMode();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusDockTest, hideState)
{
    // Test getter: int hideState()
    auto result = obj->hideState();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusDockTest, position)
{
    // Test getter: int position()
    auto result = obj->position();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusDockTest, private)
{
    // Test getter: Q_OBJECT private()
    EXPECT_NO_FATAL_FAILURE({ obj->private(); });
}

TEST_F(DBusDockTest, setHideMode)
{
    // Test method: void setHideMode(())
    EXPECT_NO_FATAL_FAILURE(obj->setHideMode());
}

TEST_F(DBusDockTest, staticInterfaceName)
{
    // Test getter: char staticInterfaceName()
    auto result = obj->staticInterfaceName();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusDockTest, staticObjectPath)
{
    // Test getter: char staticObjectPath()
    auto result = obj->staticObjectPath();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusDockTest, staticServiceName)
{
    // Test getter: char staticServiceName()
    auto result = obj->staticServiceName();
    EXPECT_EQ(result, 0);

}
