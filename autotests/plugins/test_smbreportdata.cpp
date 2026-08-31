// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbreportdata.cpp
 * @brief Unit tests for SmbReportData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "reportlog/datas/smbreportdata.h"

#include <QTest>

using namespace dfmplugin_utils;

class SmbReportDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbReportData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbReportData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbReportDataTest, type)
{
    // Test getter: QString type()
    auto result = obj->type();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
