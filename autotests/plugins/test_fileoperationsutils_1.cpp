// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperationsutils_1.cpp
 * @brief Unit tests for FileOperationsUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationutils/fileoperationsutils.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class FileOperationsUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperationsUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperationsUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperationsUtilsTest, bigFileSize)
{
    // Test getter: qint64 bigFileSize()
    auto result = obj->bigFileSize();
    EXPECT_EQ(result, 0);

}

TEST_F(FileOperationsUtilsTest, statisticsFilesSize)
{
    // Test method: SizeInfoPointer statisticsFilesSize((const QList<QUrl> &files, const bool &isRecordUrl))
    QList<QUrl> _arg0{};
    bool _arg1{};
    auto result = obj->statisticsFilesSize(_arg0, _arg1);
    EXPECT_NE(result.get(), nullptr);

}
