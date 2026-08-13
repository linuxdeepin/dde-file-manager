// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filemanagerwindowsmanager.cpp
 * @brief Unit tests for FileManagerWindowsManager (filemanagerwindowsmanager.cpp)
 *
 * FileManagerWindowsManager is a singleton that tracks file-manager windows.
 * With no windows created, the query methods return empty/zero values. No
 * real window is needed.
 */

#include <gtest/gtest.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QList>
#include <QUrl>

using namespace dfmbase;

TEST(FileManagerWindowsManagerTest, InstanceReturnsNonNullSingleton)
{
    auto &a = FileManagerWindowsManager::instance();
    auto &b = FileManagerWindowsManager::instance();
    EXPECT_EQ(&a, &b);
}

TEST(FileManagerWindowsManagerTest, WindowIdListIsEmptyBeforeAnyWindowCreated)
{
    auto &m = FileManagerWindowsManager::instance();
    EXPECT_TRUE(m.windowIdList().isEmpty());
}

TEST(FileManagerWindowsManagerTest, PreviousActivedWindowIdIsZero)
{
    auto &m = FileManagerWindowsManager::instance();
    EXPECT_EQ(m.previousActivedWindowId(), 0);
}

TEST(FileManagerWindowsManagerTest, LastActivedWindowIdIsZeroWhenEmpty)
{
    auto &m = FileManagerWindowsManager::instance();
    EXPECT_EQ(m.lastActivedWindowId(), 0);
}

TEST(FileManagerWindowsManagerTest, ResetPreviousActivedWindowIdDoesNotCrash)
{
    auto &m = FileManagerWindowsManager::instance();
    m.resetPreviousActivedWindowId();
    SUCCEED();
}

TEST(FileManagerWindowsManagerTest, ContainsCurrentUrlReturnsFalseForNoWindows)
{
    auto &m = FileManagerWindowsManager::instance();
    EXPECT_FALSE(m.containsCurrentUrl(QUrl(QStringLiteral("file:///tmp"))));
}

// --- Additional coverage for uncovered methods ---

TEST(FileManagerWindowsManagerTest, FindWindowId_NullWidgetReturnsZero)
{
    auto &m = FileManagerWindowsManager::instance();
    EXPECT_EQ(m.findWindowId(nullptr), 0);
}

TEST(FileManagerWindowsManagerTest, FindWindowById_ZeroReturnsNull)
{
    auto &m = FileManagerWindowsManager::instance();
    EXPECT_EQ(m.findWindowById(0), nullptr);
}

TEST(FileManagerWindowsManagerTest, FindWindowById_NonExistentReturnsNull)
{
    auto &m = FileManagerWindowsManager::instance();
    EXPECT_EQ(m.findWindowById(99999), nullptr);
}

TEST(FileManagerWindowsManagerTest, SetCustomWindowCreator)
{
    auto &m = FileManagerWindowsManager::instance();
    FileManagerWindowsManager::WindowCreator creator = [](const QUrl &) -> FileManagerWindowsManager::FMWindow * {
        return nullptr;
    };
    EXPECT_NO_FATAL_FAILURE({ m.setCustomWindowCreator(creator); });
}
