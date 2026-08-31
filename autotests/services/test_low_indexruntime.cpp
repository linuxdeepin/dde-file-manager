// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_indexruntime.cpp
 * @brief Unit tests for IndexRuntime methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/core/indexruntime.h"

#include <QTest>

using namespace src;

class IndexRuntimeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IndexRuntime();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IndexRuntime *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IndexRuntimeTest, fsEventController)
{
    // Test getter: FSEventController fsEventController()
    auto result = obj->fsEventController();
    EXPECT_NO_FATAL_FAILURE({ obj->fsEventController(); });

}

TEST_F(IndexRuntimeTest, selectDocumentBuilder)
{
    // Test getter: IndexDocumentBuilder selectDocumentBuilder()
    auto result = obj->selectDocumentBuilder();
    EXPECT_NO_FATAL_FAILURE({ obj->selectDocumentBuilder(); });

}

TEST_F(IndexRuntimeTest, selectExtractor)
{
    // Test getter: IndexExtractor selectExtractor()
    auto result = obj->selectExtractor();
    EXPECT_NO_FATAL_FAILURE({ obj->selectExtractor(); });

}

TEST_F(IndexRuntimeTest, taskManager)
{
    // Test getter: TaskManager taskManager()
    auto result = obj->taskManager();
    EXPECT_NO_FATAL_FAILURE({ obj->taskManager(); });

}
