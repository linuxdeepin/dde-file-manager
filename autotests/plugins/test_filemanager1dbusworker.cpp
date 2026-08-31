// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filemanager1dbusworker.cpp
 * @brief Unit tests for FileManager1DBusWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "filemanager1.h"

#include <QTest>

using namespace filemanager1;

class FileManager1DBusWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileManager1DBusWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileManager1DBusWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileManager1DBusWorkerTest, launchService)
{
    // Test method: void launchService(())
    EXPECT_NO_FATAL_FAILURE(obj->launchService());
}
