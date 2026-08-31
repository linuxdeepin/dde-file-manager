// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_baseencryptworker.cpp
 * @brief Unit tests for BaseEncryptWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/workers/baseencryptworker.h"

#include <QTest>

using namespace src;

class BaseEncryptWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BaseEncryptWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BaseEncryptWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BaseEncryptWorkerTest, args)
{
    // Test getter: QVariantMap args()
    auto result = obj->args();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseEncryptWorkerTest, exitCode)
{
    // Test getter: int exitCode()
    auto result = obj->exitCode();
    EXPECT_EQ(result, 0);

}

TEST_F(BaseEncryptWorkerTest, setExitCode)
{
    // Test setter: void setExitCode((int code))
    EXPECT_NO_FATAL_FAILURE(obj->setExitCode(0));
}
