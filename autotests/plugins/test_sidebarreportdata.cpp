// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarreportdata.cpp
 * @brief Unit tests for SidebarReportData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "reportlog/datas/sidebarreportdata.h"

#include <QTest>

using namespace dfmplugin_utils;

class SidebarReportDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SidebarReportData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SidebarReportData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SidebarReportDataTest, prepareData)
{
    // Test method: QJsonObject prepareData((const QVariantMap &args))
    QVariantMap _arg0{};
    auto result = obj->prepareData(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SidebarReportDataTest, type)
{
    // Test getter: QString type()
    auto result = obj->type();
    EXPECT_TRUE(result.isEmpty());

}
