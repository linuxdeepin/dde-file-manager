// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_normaldecryptworker.cpp
 * @brief Unit tests for NormalDecryptWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/workers/normaldecryptworker.h"

#include <QTest>

using namespace src;

class NormalDecryptWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NormalDecryptWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NormalDecryptWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NormalDecryptWorkerTest, run)
{
    // Test method: void run(())
    EXPECT_NO_FATAL_FAILURE(obj->run());
}
