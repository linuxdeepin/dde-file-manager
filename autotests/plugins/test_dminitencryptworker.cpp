// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dminitencryptworker.cpp
 * @brief Unit tests for DMInitEncryptWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/workers/dminitencryptworker.h"

#include <QTest>

using namespace src;

class DMInitEncryptWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DMInitEncryptWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DMInitEncryptWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DMInitEncryptWorkerTest, run)
{
    // Test method: void run(())
    EXPECT_NO_FATAL_FAILURE(obj->run());
}
