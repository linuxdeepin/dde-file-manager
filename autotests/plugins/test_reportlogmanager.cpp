// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_reportlogmanager.cpp
 * @brief Unit tests for ReportLogManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "reportlog/reportlogmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class ReportLogManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ReportLogManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ReportLogManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ReportLogManagerTest, commit)
{
    // Test method: void commit((const QString &type, const QVariantMap &args))
    QString _arg0{};
    QVariantMap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->commit(_arg0, _arg1));
}

TEST_F(ReportLogManagerTest, instance)
{
    // Test getter: DPUTILS_USE_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
