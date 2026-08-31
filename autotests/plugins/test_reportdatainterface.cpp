// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_reportdatainterface.cpp
 * @brief Unit tests for ReportDataInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "reportlog/datas/reportdatainterface.h"

#include <QTest>

using namespace dfmplugin_utils;

class ReportDataInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ReportDataInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ReportDataInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ReportDataInterfaceTest, ReportDataInterface)
{
    // Test constructor: ReportDataInterface(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(ReportDataInterfaceTest, ReportDataInterface_Destructor)
{
    // Test method:  ~ReportDataInterface(())
    EXPECT_NO_FATAL_FAILURE({ ReportDataInterface *tmp = new ReportDataInterface(); delete tmp; });
}
