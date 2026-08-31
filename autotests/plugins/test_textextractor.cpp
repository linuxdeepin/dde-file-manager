// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_textextractor.cpp
 * @brief Unit tests for TextExtractor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-extractor/plugins/text-extractor/textextractor.h"

#include <QTest>

using namespace src;

class TextExtractorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TextExtractor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TextExtractor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TextExtractorTest, extract)
{
    // Test method: std::optional<QByteArray> extract((const QString &filePath, size_t maxBytes))
    QString _arg0{};
    auto result = obj->extract(_arg0, {});
    EXPECT_FALSE(result.has_value());

}
