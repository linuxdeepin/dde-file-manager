// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_filemoveprocessor.cpp
 * @brief Unit tests for FileMoveProcessor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/moveprocessor.h"

#include <QTest>

using namespace src;

class FileMoveProcessorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileMoveProcessor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileMoveProcessor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileMoveProcessorTest, isFileInIndex)
{
    // Test method: bool isFileInIndex((const QString &path))
    QString _arg0{};
    auto result = obj->isFileInIndex(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileMoveProcessorTest, processContentUpdateWithCache)
{
    // Test method: bool processContentUpdateWithCache((const QString &filePath, const QString &operation))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->processContentUpdateWithCache(_arg0, _arg1);
    EXPECT_FALSE(result);

}
