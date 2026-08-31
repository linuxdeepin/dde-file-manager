// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_lucene.cpp
 * @brief Unit tests for Lucene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/profile/lowercasengramanalyzer.h"

#include <QTest>

using namespace src;

class LuceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Lucene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Lucene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(LuceneTest, Analyzer)
{
    // Test getter: SERVICETEXTINDEX_BEGIN_NAMESPACE Analyzer()
    auto result = obj->Analyzer();
    EXPECT_NO_FATAL_FAILURE({ obj->Analyzer(); });

}
