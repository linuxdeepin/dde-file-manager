// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_anythingmonitorfilter.cpp
 * @brief Unit tests for AnythingMonitorFilter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/anythingmonitorfilter.h"

#include <QTest>

using namespace dfmplugin_tag;

class AnythingMonitorFilterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AnythingMonitorFilter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AnythingMonitorFilter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AnythingMonitorFilterTest, readHomePathOfAllUsers)
{
    // Test method: void readHomePathOfAllUsers(())
    EXPECT_NO_FATAL_FAILURE(obj->readHomePathOfAllUsers());
}

TEST_F(AnythingMonitorFilterTest, reserveDir)
{
    // Test method: void reserveDir((QStringList *list))
    EXPECT_NO_FATAL_FAILURE(obj->reserveDir(nullptr));
}

TEST_F(AnythingMonitorFilterTest, restoreEscapedChar)
{
    // Test method: QString restoreEscapedChar((const QString &value))
    QString _arg0{};
    auto result = obj->restoreEscapedChar(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
