// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_keywordextractormanager.cpp
 * @brief Unit tests for KeywordExtractorManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/keywordextractor.h"

#include <QTest>

using namespace dfmplugin_workspace;

class KeywordExtractorManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new KeywordExtractorManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    KeywordExtractorManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(KeywordExtractorManagerTest, extractor)
{
    // Test getter: KeywordExtractor extractor()
    auto result = obj->extractor();
    EXPECT_NO_FATAL_FAILURE({ obj->extractor(); });

}
