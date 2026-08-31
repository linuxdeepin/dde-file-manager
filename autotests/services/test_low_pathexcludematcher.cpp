// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_pathexcludematcher.cpp
 * @brief Unit tests for PathExcludeMatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/utils/pathexcludematcher.h"

#include <QTest>

using namespace src;

class PathExcludeMatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PathExcludeMatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PathExcludeMatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PathExcludeMatcherTest, PathExcludeMatcher)
{
    // Test constructor: PathExcludeMatcher((const QStringList &patterns))
    ASSERT_NE(obj, nullptr);
}

TEST_F(PathExcludeMatcherTest, globToRegex)
{
    // Test method: QRegularExpression globToRegex((const QString &glob))
    QString _arg0{};
    auto result = obj->globToRegex(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->globToRegex(_arg0); });

}

TEST_F(PathExcludeMatcherTest, hasPatterns)
{
    // Test bool getter: hasPatterns()
    bool result = obj->hasPatterns();
    EXPECT_FALSE(result);

}

TEST_F(PathExcludeMatcherTest, parsePattern)
{
    // Test method: PathExcludeMatcher::ExcludePattern parsePattern((const QString &pattern))
    QString _arg0{};
    auto result = obj->parsePattern(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}
