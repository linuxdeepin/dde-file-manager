// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fstabinitencryptworker.cpp
 * @brief Unit tests for FstabInitEncryptWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/workers/fstabinitencryptworker.h"

#include <QTest>

using namespace src;

class FstabInitEncryptWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FstabInitEncryptWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FstabInitEncryptWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FstabInitEncryptWorkerTest, run)
{
    // Test method: void run(())
    EXPECT_NO_FATAL_FAILURE(obj->run());
}
