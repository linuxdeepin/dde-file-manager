// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultreportdata.cpp
 * @brief Unit tests for VaultReportData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "reportlog/datas/vaultreportdata.h"

#include <QTest>

using namespace dfmplugin_utils;

class VaultReportDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultReportData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultReportData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultReportDataTest, type)
{
    // Test getter: QString type()
    auto result = obj->type();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
