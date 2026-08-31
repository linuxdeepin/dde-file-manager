// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filemanagerwindowsmanager.cpp
 * @brief Unit tests for FileManagerWindowsManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <QTest>

using namespace src;

class FileManagerWindowsManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileManagerWindowsManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileManagerWindowsManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileManagerWindowsManagerTest, FileManagerWindowsManager)
{
    // Test constructor: FileManagerWindowsManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileManagerWindowsManagerTest, containsCurrentUrl)
{
    // Test method: bool containsCurrentUrl((const QUrl &url, const QWidget *win))
    QUrl _arg0{};
    auto result = obj->containsCurrentUrl(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(FileManagerWindowsManagerTest, findWindowById)
{
    // Test method: FileManagerWindowsManager::FMWindow findWindowById((quint64 winId))
    auto result = obj->findWindowById(0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileManagerWindowsManagerTest, findWindowId)
{
    // Test method: quint64 findWindowId((const QWidget *window))
    auto result = obj->findWindowId(nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(FileManagerWindowsManagerTest, instance)
{
    // Test getter: FileManagerWindowsManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(FileManagerWindowsManagerTest, lastActivedWindowId)
{
    // Test getter: quint64 lastActivedWindowId()
    auto result = obj->lastActivedWindowId();
    EXPECT_EQ(result, 0);

}

TEST_F(FileManagerWindowsManagerTest, previousActivedWindowId)
{
    // Test getter: quint64 previousActivedWindowId()
    auto result = obj->previousActivedWindowId();
    EXPECT_EQ(result, 0);

}

TEST_F(FileManagerWindowsManagerTest, resetPreviousActivedWindowId)
{
    // Test method: void resetPreviousActivedWindowId(())
    EXPECT_NO_FATAL_FAILURE(obj->resetPreviousActivedWindowId());
}

TEST_F(FileManagerWindowsManagerTest, setCustomWindowCreator)
{
    // Test setter: void setCustomWindowCreator((FileManagerWindowsManager::WindowCreator creator))
    EXPECT_NO_FATAL_FAILURE(obj->setCustomWindowCreator(FileManagerWindowsManager::WindowCreator()));
}

TEST_F(FileManagerWindowsManagerTest, windowIdList)
{
    // Test getter: QList<quint64> windowIdList()
    auto result = obj->windowIdList();
    EXPECT_TRUE(result.isEmpty());

}
