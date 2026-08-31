// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_appstartupreportdata.cpp
 * @brief Unit tests for AppStartupReportData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "reportlog/datas/appstartupreportdata.h"

#include <QTest>

using namespace dfmplugin_utils;

class AppStartupReportDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AppStartupReportData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AppStartupReportData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AppStartupReportDataTest, prepareData)
{
    // Test method: QJsonObject prepareData((const QVariantMap &args))
    QVariantMap _arg0{};
    auto result = obj->prepareData(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AppStartupReportDataTest, type)
{
    // Test getter: QString type()
    auto result = obj->type();
    EXPECT_TRUE(result.isEmpty());

}
