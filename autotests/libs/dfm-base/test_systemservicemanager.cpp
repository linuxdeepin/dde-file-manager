// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_systemservicemanager.cpp
 * @brief Unit tests for SystemServiceManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/systemservicemanager.h"

#include <QTest>

using namespace src;

class SystemServiceManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SystemServiceManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SystemServiceManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SystemServiceManagerTest, isServiceRunning)
{
    // Test method: bool isServiceRunning((const QString &serviceName))
    QString _arg0{};
    auto result = obj->isServiceRunning(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SystemServiceManagerTest, startService)
{
    // Test method: bool startService((const QString &serviceName))
    QString _arg0{};
    auto result = obj->startService(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SystemServiceManagerTest, unitPathFromName)
{
    // Test method: QString unitPathFromName((const QString &serviceName))
    QString _arg0{};
    auto result = obj->unitPathFromName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
