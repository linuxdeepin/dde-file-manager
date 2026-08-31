// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ocrextractorplugin.cpp
 * @brief Unit tests for OcrExtractorPlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-extractor/plugins/ocr-extractor/ocrextractorplugin.h"

#include <QTest>

using namespace src;

class OcrExtractorPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OcrExtractorPlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OcrExtractorPlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OcrExtractorPluginTest, extract)
{
    // Test method: std::optional<QByteArray> extract((const QString &filePath))
    QString _arg0{};
    auto result = obj->extract(_arg0);
    EXPECT_FALSE(result.has_value());

}
