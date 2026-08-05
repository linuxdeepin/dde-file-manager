// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_chinese2pinyin.cpp
 * @brief Unit tests for Pinyin::Chinese2Pinyin (chinese2pinyin.cpp)
 */

#include <gtest/gtest.h>
#include <QString>

#include <dfm-base/utils/chinese2pinyin.h>

using namespace Pinyin;

TEST(Chinese2PinyinTest, AsciiPassThrough)
{
    QString result = Chinese2Pinyin("hello");
    EXPECT_EQ(result, QString("hello"));
}

TEST(Chinese2PinyinTest, EmptyString)
{
    QString result = Chinese2Pinyin("");
    EXPECT_TRUE(result.isEmpty());
}

TEST(Chinese2PinyinTest, ChineseCharacterConverted)
{
    QString result = Chinese2Pinyin(QString::fromUtf8("中"));
    EXPECT_FALSE(result.isEmpty());
    EXPECT_NE(result, QString::fromUtf8("中"));
}

TEST(Chinese2PinyinTest, MixedContent)
{
    QString result = Chinese2Pinyin(QString::fromUtf8("a中b"));
    EXPECT_TRUE(result.startsWith('a'));
    EXPECT_TRUE(result.endsWith('b'));
    EXPECT_GT(result.size(), 2);
}

TEST(Chinese2PinyinTest, NonDictCharPassesThrough)
{
    // Emoji / rare chars not in the pinyin dict should pass through unchanged.
    QString emoji = QString::fromUtf8("\xF0\x9F\x98\x80");
    QString result = Chinese2Pinyin(emoji);
    EXPECT_EQ(result, emoji);
}
