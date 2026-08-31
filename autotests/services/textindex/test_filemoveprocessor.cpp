// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filemoveprocessor.cpp
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

TEST_F(FileMoveProcessorTest, processContentUpdate)
{
    // Test method: bool processContentUpdate((const QString &filePath))
    QString _arg0{};
    auto result = obj->processContentUpdate(_arg0);
    EXPECT_FALSE(result);

}
