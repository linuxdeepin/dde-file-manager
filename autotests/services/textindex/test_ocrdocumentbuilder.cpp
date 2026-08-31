// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ocrdocumentbuilder.cpp
 * @brief Unit tests for OcrDocumentBuilder methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/document/ocrdocumentbuilder.h"

#include <QTest>

using namespace src;

class OcrDocumentBuilderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OcrDocumentBuilder();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OcrDocumentBuilder *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OcrDocumentBuilderTest, build)
{
    // Test method: DocumentPtr build((const QString &filePath, const QString &text,
                                       const BuilderOptions &options))
    QString _arg0{};
    QString _arg1{};
    BuilderOptions _arg2{};
    auto result = obj->build(_arg0, _arg1, _arg2);
    EXPECT_NE(result.get(), nullptr);

}
