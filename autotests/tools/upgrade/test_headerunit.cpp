// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/headerunit.h"

#include <gtest/gtest.h>
#include <QMap>

using namespace dfm_upgrade;

// Tests for units that don't require file system setup
class TestHeaderUnit : public testing::Test {
};

TEST_F(TestHeaderUnit, name)
{
    HeaderUnit unit;
    EXPECT_EQ(unit.name(), QString("Header"));
}

TEST_F(TestHeaderUnit, initialize)
{
    HeaderUnit unit;
    QMap<QString, QString> args;
    bool result = unit.initialize(args);
    EXPECT_TRUE(result);
}

TEST_F(TestHeaderUnit, upgrade)
{
    HeaderUnit unit;
    QMap<QString, QString> args;
    unit.initialize(args);
    bool result = unit.upgrade();
    EXPECT_TRUE(result);
}