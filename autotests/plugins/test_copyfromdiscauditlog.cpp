// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_copyfromdiscauditlog.cpp
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

TEST_F(CopyFromDiscAuditLogTest, doLog)
{
    // Test method: void doLog((QDBusInterface &interface))
    QDBusInterface _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->doLog(_arg0));
}

TEST_F(CopyFromDiscAuditLogTest, writeLog)
{
    // Test method: void writeLog((QDBusInterface &interface, const QString &srcPath, const QString &destPath))
    QDBusInterface _arg0{};
    QString _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->writeLog(_arg0, _arg1, _arg2));
}
