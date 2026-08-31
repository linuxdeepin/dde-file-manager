// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_displayconfig.cpp
 * @brief Unit tests for DisplayConfig methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displayconfig.h"

#include <QTest>

using namespace ddplugin_canvas;

class DisplayConfigTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DisplayConfig();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DisplayConfig *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DisplayConfigTest, path)
{
    // Test getter: QString path()
    auto result = obj->path();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DisplayConfigTest, profile)
{
    // Test getter: QList<QString> profile()
    auto result = obj->profile();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DisplayConfigTest, remove)
{
    // Test method: void remove((const QString &group, const QStringList &keys))
    QString _arg0{};
    QStringList _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->remove(_arg0, _arg1));
}

TEST_F(DisplayConfigTest, value)
{
    // Test method: QVariant value((const QString &group, const QString &key, const QVariant &defaultVar))
    QString _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    auto result = obj->value(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result.isValid());

}
