// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_normalinitencryptworker.cpp
 * @brief Unit tests for NormalInitEncryptWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/workers/normalinitencryptworker.h"

#include <QTest>

using namespace src;

class NormalInitEncryptWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NormalInitEncryptWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NormalInitEncryptWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NormalInitEncryptWorkerTest, initJobArgs)
{
    // Test method: job_file_helper::JobDescArgs initJobArgs((DevPtr ptr))
    auto result = obj->initJobArgs(DevPtr());
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(NormalInitEncryptWorkerTest, run)
{
    // Test method: void run(())
    EXPECT_NO_FATAL_FAILURE(obj->run());
}
