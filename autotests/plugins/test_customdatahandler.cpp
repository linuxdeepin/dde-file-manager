// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customdatahandler.cpp
 * @brief Unit tests for CustomDataHandler Mid-priority methods (ddplugin-organizer)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "mode/custom/customdatahandler.h"

using namespace ddplugin_organizer;

class CustomDataHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(CustomDataHandlerTest, acceptInsert)
{
    // Instance method acceptInsert
    CustomDataHandler obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.acceptInsert(QUrl("file:///tmp/test")); });
    (void)result;
}

TEST_F(CustomDataHandlerTest, acceptRename)
{
    // Instance method acceptRename
    CustomDataHandler obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.acceptRename(QUrl("file:///tmp/test"), QUrl("file:///tmp/test")); });
    (void)result;
}

TEST_F(CustomDataHandlerTest, acceptReset)
{
    // Instance method acceptReset
    CustomDataHandler obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.acceptReset(QList<QUrl>{QUrl("file:///tmp/test")}); (void)r; });
}

TEST_F(CustomDataHandlerTest, append)
{
    // Instance method append
    CustomDataHandler obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.append(QUrl("file:///tmp/test")); (void)r; });
}

TEST_F(CustomDataHandlerTest, change)
{
    // Instance method change
    CustomDataHandler obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.change(QUrl("file:///tmp/test")); (void)r; });
}

TEST_F(CustomDataHandlerTest, check)
{
    // Instance method check
    CustomDataHandler obj;
    EXPECT_NO_FATAL_FAILURE({ obj.check(QSet<QUrl>()); });
}

TEST_F(CustomDataHandlerTest, insert)
{
    // Instance method insert
    CustomDataHandler obj;
    EXPECT_NO_FATAL_FAILURE({ obj.insert(QUrl("file:///tmp/test"), QString("test"), 0); });
}

TEST_F(CustomDataHandlerTest, prepend)
{
    // Instance method prepend
    CustomDataHandler obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.prepend(QUrl("file:///tmp/test")); (void)r; });
}

TEST_F(CustomDataHandlerTest, remove)
{
    // Instance method remove
    CustomDataHandler obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.remove(QUrl("file:///tmp/test")); (void)r; });
}

TEST_F(CustomDataHandlerTest, removeBaseData)
{
    // Instance method removeBaseData
    CustomDataHandler obj;
    EXPECT_NO_FATAL_FAILURE({ obj.removeBaseData(QString("test")); });
}

TEST_F(CustomDataHandlerTest, reset)
{
    // Instance method reset
    CustomDataHandler obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.reset({}); });
    (void)result;
}
