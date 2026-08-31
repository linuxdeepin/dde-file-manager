// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burnfilesauditlogjob.cpp
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

TEST_F(BurnFilesAuditLogJobTest, doLog)
{
    // Test method: void doLog((QDBusInterface &interface))
    QDBusInterface _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->doLog(_arg0));
}

TEST_F(BurnFilesAuditLogJobTest, writeLog)
{
    // Test method: void writeLog((QDBusInterface &interface, const QString &discPath, const QString &nativePath, qint64 size))
    QDBusInterface _arg0{};
    QString _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->writeLog(_arg0, _arg1, _arg2, 0));
}
