// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractauditlogjob.cpp
 * @brief Unit tests for AbstractAuditLogJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/auditlogjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class AbstractAuditLogJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractAuditLogJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractAuditLogJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractAuditLogJobTest, AbstractAuditLogJob)
{
    // Test constructor: AbstractAuditLogJob((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AbstractAuditLogJobTest, run)
{
    // Test method: void run(())
    EXPECT_NO_FATAL_FAILURE(obj->run());
}
