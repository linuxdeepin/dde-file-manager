// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_contentdocumentbuilder.cpp
 * @brief Unit tests for ContentDocumentBuilder methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/document/contentdocumentbuilder.h"

#include <QTest>

using namespace src;

class ContentDocumentBuilderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ContentDocumentBuilder();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ContentDocumentBuilder *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ContentDocumentBuilderTest, build)
{
    // Test method: DocumentPtr build((const QString &filePath, const QString &text,
                                          const BuilderOptions &options))
    QString _arg0{};
    QString _arg1{};
    BuilderOptions _arg2{};
    auto result = obj->build(_arg0, _arg1, _arg2);
    EXPECT_NE(result.get(), nullptr);

}
