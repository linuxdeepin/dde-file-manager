// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_desktopstartupreportdata.cpp
 * @brief Unit tests for DesktopStartUpReportData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "reportlog/datas/desktopstartupreportdata.h"

#include <QTest>

using namespace dfmplugin_utils;

class DesktopStartUpReportDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DesktopStartUpReportData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DesktopStartUpReportData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DesktopStartUpReportDataTest, prepareData)
{
    // Test method: QJsonObject prepareData((const QVariantMap &args))
    QVariantMap _arg0{};
    auto result = obj->prepareData(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DesktopStartUpReportDataTest, type)
{
    // Test getter: QString type()
    auto result = obj->type();
    EXPECT_TRUE(result.isEmpty());

}
