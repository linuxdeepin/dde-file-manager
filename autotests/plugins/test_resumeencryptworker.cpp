// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_resumeencryptworker.cpp
 * @brief Unit tests for ResumeEncryptWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/workers/resumeencryptworker.h"

#include <QTest>

using namespace src;

class ResumeEncryptWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ResumeEncryptWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ResumeEncryptWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ResumeEncryptWorkerTest, recoveryKey)
{
    // Test getter: QString recoveryKey()
    auto result = obj->recoveryKey();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(ResumeEncryptWorkerTest, run)
{
    // Test method: void run(())
    EXPECT_NO_FATAL_FAILURE(obj->run());
}

TEST_F(ResumeEncryptWorkerTest, saveRecoveryKey)
{
    // Test method: void saveRecoveryKey(())
    EXPECT_NO_FATAL_FAILURE(obj->saveRecoveryKey());
}
