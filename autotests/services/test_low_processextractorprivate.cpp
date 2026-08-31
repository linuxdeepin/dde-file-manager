// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_processextractorprivate.cpp
 * @brief Unit tests for ProcessExtractorPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/extractor/processextractor.h"

#include <QTest>

using namespace src;

class ProcessExtractorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProcessExtractorPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProcessExtractorPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProcessExtractorPrivateTest, ProcessExtractorPrivate)
{
    // Test constructor: ProcessExtractorPrivate(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(ProcessExtractorPrivateTest, ProcessExtractorPrivate_Destructor)
{
    // Test method:  ~ProcessExtractorPrivate(())
    EXPECT_NO_FATAL_FAILURE({ ProcessExtractorPrivate *tmp = new ProcessExtractorPrivate(); delete tmp; });
}
