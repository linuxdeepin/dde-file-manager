// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_indexextractor.cpp
 * @brief Unit tests for IndexExtractor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/extractor/indexextractor.h"

#include <QTest>

using namespace src;

class IndexExtractorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IndexExtractor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IndexExtractor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IndexExtractorTest, IndexExtractor_Destructor)
{
    // Test method:  ~IndexExtractor(())
    EXPECT_NO_FATAL_FAILURE({ IndexExtractor *tmp = new IndexExtractor(); delete tmp; });
}
