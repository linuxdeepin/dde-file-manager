// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileshredworker.cpp
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

TEST_F(FileShredWorkerTest, FileShredWorker)
{
    // Test constructor: FileShredWorker((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileShredWorkerTest, countFilesInDirectory)
{
    // Test method: int countFilesInDirectory((const QString &dirPath))
    QString _arg0{};
    auto result = obj->countFilesInDirectory(_arg0);
    EXPECT_EQ(result, 0);

}

TEST_F(FileShredWorkerTest, parseShredOutput)
{
    // Test method: void parseShredOutput((const QString &output))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->parseShredOutput(_arg0));
}
