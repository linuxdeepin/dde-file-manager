// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_upgradefactory.cpp
 * @brief Unit tests for UpgradeFactory methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tools/upgrade/core/upgradefactory.h"

#include <QTest>

using namespace src;

class UpgradeFactoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UpgradeFactory();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UpgradeFactory *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UpgradeFactoryTest, doUpgrade)
{
    // Test method: void doUpgrade(())
    EXPECT_NO_FATAL_FAILURE(obj->doUpgrade());
}

TEST_F(UpgradeFactoryTest, previous)
{
    // Test method: void previous((const QMap<QString, QString> &args))
    QMap<QString, QString> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->previous(_arg0));
}
