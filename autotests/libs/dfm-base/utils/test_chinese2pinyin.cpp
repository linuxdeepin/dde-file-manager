// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>

#include <dfm-base/utils/chinese2pinyin.h>
#include "stubext.h"

using namespace Pinyin;

class Chinese2PinyinTest : public ::testing::Test {
protected:
    void SetUp() override {
        stub.clear();
        // Initialize test object
    }

    void TearDown() override {
        stub.clear();
        // Cleanup resources
    }

    stub_ext::StubExt stub;
};

TEST_F(Chinese2PinyinTest, Chinese2Pinyin_BasicConversion_ExpectedResult) {
    // Arrange
    QString input = QStringLiteral("你好");

    // Act
    QString result = Pinyin::Chinese2Pinyin(input);

    // Assert
    EXPECT_FALSE(result.isEmpty());
    // The result should contain pinyin for Chinese characters
    EXPECT_TRUE(result.contains(QLatin1String("ni")));
    EXPECT_TRUE(result.contains(QLatin1String("hao")));
}

TEST_F(Chinese2PinyinTest, Chinese2Pinyin_EmptyInput_ExpectedEmptyResult) {
    // Arrange
    QString input = "";

    // Act
    QString result = Pinyin::Chinese2Pinyin(input);

    // Assert
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(Chinese2PinyinTest, Chinese2Pinyin_EnglishInput_ExpectedSameResult) {
    // Arrange
    QString input = QStringLiteral("hello");

    // Act
    QString result = Pinyin::Chinese2Pinyin(input);

    // Assert
    EXPECT_EQ(result, input);
}

TEST_F(Chinese2PinyinTest, Chinese2Pinyin_MixedText_ExpectedMixedResult) {
    // Arrange
    QString input = QStringLiteral("Hello世界");

    // Act
    QString result = Pinyin::Chinese2Pinyin(input);

    // Assert
    // Result should contain English part unchanged and Chinese converted to pinyin
    EXPECT_TRUE(result.contains(QLatin1String("Hello")));
    EXPECT_TRUE(result.contains(QLatin1String("shi"))); // shi is pinyin for 世
    EXPECT_TRUE(result.contains(QLatin1String("jie"))); // jie is pinyin for 界
}

TEST_F(Chinese2PinyinTest, Chinese2Pinyin_SpecialCharactersAndNumbers_ExpectedHandledCorrectly) {
    // Arrange
    QString input = QStringLiteral("测试123!@#");

    // Act
    QString result = Pinyin::Chinese2Pinyin(input);

    // Assert
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains(QLatin1String("123")));
    EXPECT_TRUE(result.contains(QLatin1String("ce")));  // ce is pinyin for 测
    EXPECT_TRUE(result.contains(QLatin1String("shi"))); // shi is pinyin for 试
}

TEST_F(Chinese2PinyinTest, Chinese2Pinyin_Punctuation_ExpectedHandledCorrectly) {
    // Arrange
    QString input = QStringLiteral("测试。");

    // Act
    QString result = Pinyin::Chinese2Pinyin(input);

    // Assert
    EXPECT_FALSE(result.isEmpty());
    // Check that punctuation is preserved and Chinese is converted to pinyin
    EXPECT_TRUE(result.contains(QLatin1String("ce")));
    EXPECT_TRUE(result.contains(QLatin1String("shi")));
}

TEST_F(Chinese2PinyinTest, Chinese2Pinyin_LongText_ExpectedConvertedSuccessfully) {
    // Arrange
    QString input = QStringLiteral("中华人民共和国");

    // Act
    QString result = Pinyin::Chinese2Pinyin(input);

    // Assert
    EXPECT_FALSE(result.isEmpty());
    // Check for some expected pinyin parts
    EXPECT_TRUE(result.contains(QLatin1String("zhong")));
    EXPECT_TRUE(result.contains(QLatin1String("hua")));
    EXPECT_TRUE(result.contains(QLatin1String("ren")));
    EXPECT_TRUE(result.contains(QLatin1String("min")));
}
