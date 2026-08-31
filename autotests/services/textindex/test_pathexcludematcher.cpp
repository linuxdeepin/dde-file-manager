// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pathexcludematcher.cpp
 * @brief Unit tests for PathExcludeMatcher (pathexcludematcher.cpp)
 */

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/utils/pathexcludematcher.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

TEST(PathExcludeMatcherTest, DefaultConstructorEmpty)
{
    PathExcludeMatcher matcher;
    EXPECT_FALSE(matcher.hasPatterns());
    EXPECT_EQ(matcher.patternCount(), 0);
}

TEST(PathExcludeMatcherTest, ConstructWithPatterns)
{
    PathExcludeMatcher matcher({ "tmp", "/home/user/exclude" });
    EXPECT_TRUE(matcher.hasPatterns());
    EXPECT_EQ(matcher.patternCount(), 2);
}

TEST(PathExcludeMatcherTest, AddPatternDuplicateIgnored)
{
    PathExcludeMatcher matcher;
    matcher.addPattern("tmp");
    matcher.addPattern("tmp");
    EXPECT_EQ(matcher.patternCount(), 1);
}

TEST(PathExcludeMatcherTest, AddEmptyPatternIgnored)
{
    PathExcludeMatcher matcher;
    matcher.addPattern("");
    EXPECT_FALSE(matcher.hasPatterns());
}

TEST(PathExcludeMatcherTest, AddPatternsList)
{
    PathExcludeMatcher matcher;
    matcher.addPatterns({ "a", "b", "c" });
    EXPECT_EQ(matcher.patternCount(), 3);
}

TEST(PathExcludeMatcherTest, RemovePattern)
{
    PathExcludeMatcher matcher({ "a", "b", "c" });
    matcher.removePattern("b");
    EXPECT_EQ(matcher.patternCount(), 2);
    EXPECT_FALSE(matcher.patterns().contains("b"));
}

TEST(PathExcludeMatcherTest, Clear)
{
    PathExcludeMatcher matcher({ "a", "b" });
    matcher.clear();
    EXPECT_FALSE(matcher.hasPatterns());
    EXPECT_EQ(matcher.patternCount(), 0);
}

TEST(PathExcludeMatcherTest, ShouldExcludeEmptyPathReturnsFalse)
{
    PathExcludeMatcher matcher({ "tmp" });
    EXPECT_FALSE(matcher.shouldExclude(""));
}

TEST(PathExcludeMatcherTest, ShouldExcludeNoPatternsReturnsFalse)
{
    PathExcludeMatcher matcher;
    EXPECT_FALSE(matcher.shouldExclude("/home/user/tmp"));
}

TEST(PathExcludeMatcherTest, ExactNameMatch)
{
    PathExcludeMatcher matcher({ "tmp" });
    EXPECT_TRUE(matcher.shouldExclude("/home/user/tmp"));
    EXPECT_TRUE(matcher.shouldExclude("/home/user/tmp/subdir"));
    EXPECT_FALSE(matcher.shouldExclude("/home/user/template"));
}

TEST(PathExcludeMatcherTest, AbsolutePrefixMatch)
{
    PathExcludeMatcher matcher({ "/home/user/exclude" });
    EXPECT_TRUE(matcher.shouldExclude("/home/user/exclude"));
    EXPECT_TRUE(matcher.shouldExclude("/home/user/exclude/subdir"));
    EXPECT_FALSE(matcher.shouldExclude("/home/user/other"));
}

TEST(PathExcludeMatcherTest, GlobPatternMatch)
{
    PathExcludeMatcher matcher({ "build-*" });
    EXPECT_TRUE(matcher.shouldExclude("/home/user/build-debug"));
    EXPECT_FALSE(matcher.shouldExclude("/home/user/src"));
}

TEST(PathExcludeMatcherTest, PathSegmentMatch)
{
    PathExcludeMatcher matcher({ ".local/share/Trash" });
    EXPECT_TRUE(matcher.shouldExclude("/.local/share/Trash"));
    EXPECT_TRUE(matcher.shouldExclude("/.local/share/Trash/sub"));
    EXPECT_FALSE(matcher.shouldExclude("/.local/share/Trashcan"));
}

TEST(PathExcludeMatcherTest, PatternsListReturnsOriginals)
{
    PathExcludeMatcher matcher({ "tmp", "/abs/path" });
    QStringList pats = matcher.patterns();
    EXPECT_TRUE(pats.contains("tmp"));
    EXPECT_TRUE(pats.contains("/abs/path"));
}

TEST(PathExcludeMatcherTest, CreateForIndexReturnsMatcher)
{
    PathExcludeMatcher matcher = PathExcludeMatcher::createForIndex();
    EXPECT_NO_FATAL_FAILURE({ matcher.hasPatterns(); });
}

TEST(PathExcludeMatcherTest, GlobToRegexSimple)
{
    QRegularExpression re = PathExcludeMatcher::globToRegex("*.txt");
    EXPECT_TRUE(re.match("a.txt").hasMatch());
    EXPECT_FALSE(re.match("a.tx").hasMatch());
}

TEST(PathExcludeMatcherTest, GlobToRegexQuestionMark)
{
    QRegularExpression re = PathExcludeMatcher::globToRegex("a?c");
    EXPECT_TRUE(re.match("abc").hasMatch());
    EXPECT_FALSE(re.match("ac").hasMatch());
}

TEST(PathExcludeMatcherTest, GlobToRegexEscapesSpecialChars)
{
    QRegularExpression re = PathExcludeMatcher::globToRegex("a.b");
    EXPECT_TRUE(re.match("a.b").hasMatch());
    EXPECT_FALSE(re.match("axb").hasMatch());
}


TEST(PathExcludeMatcherTest, addPattern)
{
    PathExcludeMatcher obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj.addPattern(_arg0));
}

TEST(PathExcludeMatcherTest, addPatterns)
{
    PathExcludeMatcher obj;
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj.addPatterns(_arg0));
}

TEST(PathExcludeMatcherTest, clear)
{
    PathExcludeMatcher obj;
    EXPECT_NO_FATAL_FAILURE(obj.clear());
}

TEST(PathExcludeMatcherTest, createForIndex)
{
    PathExcludeMatcher obj;
    EXPECT_NO_FATAL_FAILURE({ obj.createForIndex(); });
}

TEST(PathExcludeMatcherTest, patternCount)
{
    PathExcludeMatcher obj;
    EXPECT_NO_FATAL_FAILURE({ obj.patternCount(); });
}

TEST(PathExcludeMatcherTest, patterns)
{
    PathExcludeMatcher obj;
    EXPECT_NO_FATAL_FAILURE({ obj.patterns(); });
}

TEST(PathExcludeMatcherTest, removePattern)
{
    PathExcludeMatcher obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj.removePattern(_arg0));
}

TEST(PathExcludeMatcherTest, shouldExclude)
{
    PathExcludeMatcher obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.shouldExclude(_arg0); });
}
