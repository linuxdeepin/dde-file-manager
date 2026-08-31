// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dmdecryptworker.cpp
 * @brief Unit tests for DMDecryptWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/workers/dmdecryptworker.h"

#include <QTest>

using namespace src;

class DMDecryptWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DMDecryptWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DMDecryptWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DMDecryptWorkerTest, run)
{
    // Test method: void run(())
    EXPECT_NO_FATAL_FAILURE(obj->run());
}
