// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dm_setup.cpp
 * @brief Unit tests for dm_setup methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/core/dmsetup.h"

#include <QTest>

using namespace src;

class dm_setupTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new dm_setup();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    dm_setup *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(dm_setupTest, dmRemoveDevice)
{
    // Test method: int dmRemoveDevice((const QString &dmDev))
    QString _arg0{};
    auto result = obj->dmRemoveDevice(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(dm_setupTest, dmResumeDevice)
{
    // Test method: int dmResumeDevice((const QString &dmDev))
    QString _arg0{};
    auto result = obj->dmResumeDevice(_arg0);
    EXPECT_GE(result, 0);

}
