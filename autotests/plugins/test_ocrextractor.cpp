// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ocrextractor.cpp
 * @brief Unit tests for OcrExtractor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-extractor/plugins/ocr-extractor/ocrextractor.h"

#include <QTest>

using namespace src;

class OcrExtractorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OcrExtractor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OcrExtractor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OcrExtractorTest, extract)
{
    // Test method: std::optional<QByteArray> extract((const QString &filePath))
    QString _arg0{};
    auto result = obj->extract(_arg0);
    EXPECT_FALSE(result.has_value());

}
