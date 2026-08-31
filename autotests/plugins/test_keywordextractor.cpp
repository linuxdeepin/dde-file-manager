// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_keywordextractor.cpp
 * @brief Unit tests for KeywordExtractor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/keywordextractor.h"

#include <QTest>

using namespace dfmplugin_workspace;

class KeywordExtractorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new KeywordExtractor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    KeywordExtractor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(KeywordExtractorTest, extractFromKeyword)
{
    // Test method: QStringList extractFromKeyword((const QString &keyword))
    QString _arg0{};
    auto result = obj->extractFromKeyword(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(KeywordExtractorTest, extractFromUrl)
{
    // Test method: QStringList extractFromUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->extractFromUrl(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(KeywordExtractorTest, registerStrategy)
{
    // Test method: void registerStrategy((QSharedPointer<KeywordExtractionStrategy> strategy))
    EXPECT_NO_FATAL_FAILURE(obj->registerStrategy(QSharedPointer<KeywordExtractionStrategy>()));
}
