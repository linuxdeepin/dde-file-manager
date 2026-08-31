// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_loggerrules.cpp
 * @brief Unit tests for LoggerRules (utils/loggerrules.cpp)
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QString>
#include <QStringList>

#include <dfm-base/utils/loggerrules.h>

using namespace dfmbase;

TEST(LoggerRulesTest, InstanceReturnsSameReference)
{
    LoggerRules &a = LoggerRules::instance();
    LoggerRules &b = LoggerRules::instance();
    EXPECT_EQ(&a, &b);
}

TEST(LoggerRulesTest, SetRulesConvertsSemicolonsToNewlines)
{
    LoggerRules &rules = LoggerRules::instance();
    rules.setRules("dfm.foo.debug=true;dfm.bar.debug=false");
    const QString current = rules.rules();
    EXPECT_FALSE(current.contains(';'));
    EXPECT_TRUE(current.contains("dfm.foo.debug=true"));
    EXPECT_TRUE(current.contains("dfm.bar.debug=false"));
    EXPECT_TRUE(current.contains('\n'));
}

TEST(LoggerRulesTest, SetRulesSingleRuleHasNoNewline)
{
    LoggerRules &rules = LoggerRules::instance();
    rules.setRules("single.rule=true");
    EXPECT_EQ(rules.rules(), QString("single.rule=true"));
}

TEST(LoggerRulesTest, InitLoggerRulesReadsEnvAndAppends)
{
    // Provide an explicit logging-rules environment; initLoggerRules reads
    // QT_LOGGING_RULES first, then appends DConfig-provided rules on top.
    qputenv("QT_LOGGING_RULES", "dfm.envtest.debug=true");
    LoggerRules &rules = LoggerRules::instance();
    rules.initLoggerRules();
    const QString current = rules.rules();
    EXPECT_TRUE(current.contains("dfm.envtest.debug=true"));
}

TEST(LoggerRulesTest, InitLoggerRulesIsIdempotentSafe)
{
    LoggerRules &rules = LoggerRules::instance();
    rules.setRules("dfm.idempotent.debug=true");
    rules.initLoggerRules();
    QString after1 = rules.rules();
    rules.initLoggerRules();
    QString after2 = rules.rules();
    // Idempotent: calling initLoggerRules twice should not duplicate rules.
    EXPECT_EQ(after1, after2);
}

TEST(LoggerRulesTest, RulesAccessorReturnsString)
{
    LoggerRules &rules = LoggerRules::instance();
    rules.setRules("accessor.test=true");
    EXPECT_EQ(rules.rules().toStdString(), "accessor.test=true");
}


TEST(LoggerRulesTest, appendRules)
{
    LoggerRules obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj.appendRules(_arg0));
}

TEST(LoggerRulesTest, initLoggerRules)
{
    LoggerRules obj;
    EXPECT_NO_FATAL_FAILURE(obj.initLoggerRules());
}

TEST(LoggerRulesTest, rules)
{
    LoggerRules obj;
    EXPECT_NO_FATAL_FAILURE({ obj.rules(); });
}

TEST(LoggerRulesTest, setRules)
{
    LoggerRules obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj.setRules(_arg0));
}
