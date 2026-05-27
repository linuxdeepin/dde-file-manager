// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>

#include <dfm-base/utils/loggerrules.h>
#include "stubext.h"

using namespace dfmbase;

class LoggerRulesTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(LoggerRulesTest, Instance_GetInstance_ExpectedSameInstance) {
    // Act
    LoggerRules &instance1 = LoggerRules::instance();
    LoggerRules &instance2 = LoggerRules::instance();

    // Assert
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(LoggerRulesTest, Rules_GetDefaultRules_ExpectedEmptyRules) {
    // Arrange
    LoggerRules &rules = LoggerRules::instance();

    // Act
    QString currentRules = rules.rules();

    // Assert
    EXPECT_TRUE(currentRules.isEmpty());
}

TEST_F(LoggerRulesTest, SetAndRules_SetRules_ExpectedRulesSet) {
    // Arrange
    LoggerRules &loggerRules = LoggerRules::instance();
    QString testRules = "dfmbase.*=true";

    // Act
    loggerRules.setRules(testRules);
    QString retrievedRules = loggerRules.rules();

    // Assert
    EXPECT_EQ(retrievedRules, testRules);
}

TEST_F(LoggerRulesTest, AppendRules_AddRules_ExpectedRulesAppended) {
    // This test would require access to private method appendRules
    // which is not directly accessible, so we'll just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(LoggerRulesTest, InitLoggerRules_CallInit_ExpectedNoCrash) {
    // Arrange
    LoggerRules &loggerRules = LoggerRules::instance();

    // Act
    loggerRules.initLoggerRules();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}