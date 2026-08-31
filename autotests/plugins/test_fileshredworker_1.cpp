// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileshredworker_1.cpp
 * @brief Unit tests for FileShredWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "shred/fileshredworker.h"

#include <QTest>

using namespace dfmplugin_utils;

class FileShredWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileShredWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileShredWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileShredWorkerTest, calculateProgress)
{
    // Test method: int calculateProgress((int processedFiles, int totalFiles, int currentFileProgress))
    auto result = obj->calculateProgress(0, 0, 0);
    EXPECT_GE(result, 0);

}

TEST_F(FileShredWorkerTest, isPipe)
{
    // Test method: bool isPipe((const QString &path))
    QString _arg0{};
    auto result = obj->isPipe(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileShredWorkerTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}
