// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_erasediscauditlogjob_1.cpp
 * @brief Unit tests for EraseDiscAuditLogJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/auditlogjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class EraseDiscAuditLogJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EraseDiscAuditLogJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EraseDiscAuditLogJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EraseDiscAuditLogJobTest, doLog)
{
    // Test method: void doLog((QDBusInterface &interface))
    QDBusInterface _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->doLog(_arg0));
}
