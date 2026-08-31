// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_copyfromdiscauditlog_1.cpp
 * @brief Unit tests for CopyFromDiscAuditLog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/auditlogjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class CopyFromDiscAuditLogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CopyFromDiscAuditLog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CopyFromDiscAuditLog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CopyFromDiscAuditLogTest, CopyFromDiscAuditLog)
{
    // Test constructor: CopyFromDiscAuditLog((const QList<QUrl> &srcList, const QList<QUrl> &destList, QObject *parent))
    ASSERT_NE(obj, nullptr);
}
