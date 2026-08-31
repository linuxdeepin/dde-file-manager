// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbreportdata_1.cpp
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

TEST_F(SmbReportDataTest, prepareData)
{
    // Test method: QJsonObject prepareData((const QVariantMap &args))
    QVariantMap _arg0{};
    auto result = obj->prepareData(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
