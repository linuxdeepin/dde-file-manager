// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
#include <QUrl>

using namespace dfm_upgrade;

// Add test for base UpgradeUnit completed() method
class TestUpgradeUnitBase : public testing::Test {
};

TEST_F(TestUpgradeUnitBase, completed)
{
    // Test the base class completed method by creating a concrete implementation
    class TestUnit : public dfm_upgrade::UpgradeUnit
    {
    public:
        QString name() override { return "TestUnit"; }
        bool initialize(const QMap<QString, QString> &args) override { Q_UNUSED(args); return true; }
        bool upgrade() override { return true; }
    };

    TestUnit unit;
    unit.completed();  // Should not crash or throw
    SUCCEED();  // Just verify that the method can be called without issues
}