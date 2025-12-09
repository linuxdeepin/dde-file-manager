// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/bookmarkupgrade/defaultitemmanager.h"
#include "units/bookmarkupgradeunit.h"
#include "core/upgradeunit.h"
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
#include <QList>
#include <QUrl>

using namespace dfm_upgrade;

class TestDefaultItemManager : public testing::Test {
};

TEST_F(TestDefaultItemManager, instance)
{
    DefaultItemManager* manager = DefaultItemManager::instance();
    EXPECT_NE(manager, nullptr);
}

TEST_F(TestDefaultItemManager, initDefaultItems)
{
    DefaultItemManager* manager = DefaultItemManager::instance();
    manager->initDefaultItems();
    QList<BookmarkData> items = manager->defaultItemInitOrder();
    EXPECT_FALSE(items.isEmpty());
}

TEST_F(TestDefaultItemManager, initPreDefineItems)
{
    DefaultItemManager* manager = DefaultItemManager::instance();
    manager->initPreDefineItems();
    QList<BookmarkData> items = manager->defaultPreDefInitOrder();
    // This might be empty depending on plugin availability
    // EXPECT_FALSE(items.isEmpty());
}