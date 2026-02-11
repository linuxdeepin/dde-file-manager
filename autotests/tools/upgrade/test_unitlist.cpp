// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/unitlist.h"
#include "core/upgradeunit.h"

#include "stubext.h"

#include <gtest/gtest.h>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDateTime>
#include <QMap>

using namespace dfm_upgrade;

class TestUnitList : public testing::Test {
};

TEST_F(TestUnitList, createUnits)
{
    auto units = createUnits();
    EXPECT_FALSE(units.isEmpty());
    EXPECT_GT(units.size(), 0);
}