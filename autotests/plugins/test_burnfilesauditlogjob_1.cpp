// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burnfilesauditlogjob_1.cpp
 * @brief Unit tests for BurnFilesAuditLogJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/auditlogjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class BurnFilesAuditLogJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BurnFilesAuditLogJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BurnFilesAuditLogJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BurnFilesAuditLogJobTest, BurnFilesAuditLogJob)
{
    // Test constructor: BurnFilesAuditLogJob((const QUrl &stagingUrl, bool result, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BurnFilesAuditLogJobTest, burnedFileInfoList)
{
    // Test getter: QFileInfoList burnedFileInfoList()
    auto result = obj->burnedFileInfoList();
    EXPECT_TRUE(result.isEmpty());

}
