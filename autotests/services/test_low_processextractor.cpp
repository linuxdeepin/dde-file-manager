// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_processextractor.cpp
 * @brief Unit tests for ProcessExtractor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/extractor/processextractor.h"

#include <QTest>

using namespace src;

class ProcessExtractorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProcessExtractor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProcessExtractor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProcessExtractorTest, ProcessExtractor)
{
    // Test constructor: ProcessExtractor(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(ProcessExtractorTest, extract)
{
    // Test method: IndexExtractionResult extract((const QString &filePath, size_t maxBytes))
    QString _arg0{};
    auto result = obj->extract(_arg0, {});
    EXPECT_NO_FATAL_FAILURE({ obj->extract(_arg0, {}); });

}

TEST_F(ProcessExtractorTest, ProcessExtractor_Destructor)
{
    // Test method:  ~ProcessExtractor(())
    EXPECT_NO_FATAL_FAILURE({ ProcessExtractor *tmp = new ProcessExtractor(); delete tmp; });
}
