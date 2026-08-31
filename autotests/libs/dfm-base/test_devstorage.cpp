// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devstorage.cpp
 * @brief Unit tests for DevStorage methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/device/private/devicewatcher_p.h"

#include <QTest>

using namespace src;

class DevStorageTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DevStorage();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DevStorage *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DevStorageTest, isValid)
{
    // Test bool getter: isValid()
    bool result = obj->isValid();
    EXPECT_FALSE(result);

}
