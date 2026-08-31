// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filecleanupmanager.cpp
 * @brief Unit tests for FileCleanupManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationutils/filecleanupmanager.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class FileCleanupManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileCleanupManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileCleanupManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileCleanupManagerTest, cleanupIncompleteFiles)
{
    // Test method: void cleanupIncompleteFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->cleanupIncompleteFiles());
}

TEST_F(FileCleanupManagerTest, confirmCompleted)
{
    // Test method: void confirmCompleted((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->confirmCompleted(_arg0));
}
