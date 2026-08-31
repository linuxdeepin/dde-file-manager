// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filemenureportdata.cpp
 * @brief Unit tests for FileMenuReportData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "reportlog/datas/filemenureportdata.h"

#include <QTest>

using namespace dfmplugin_utils;

class FileMenuReportDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileMenuReportData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileMenuReportData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileMenuReportDataTest, prepareData)
{
    // Test method: QJsonObject prepareData((const QVariantMap &args))
    QVariantMap _arg0{};
    auto result = obj->prepareData(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileMenuReportDataTest, type)
{
    // Test getter: QString type()
    auto result = obj->type();
    EXPECT_TRUE(result.isEmpty());

}
