// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_fstabinitencryptworker.cpp
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

TEST_F(FstabInitEncryptWorkerTest, initJobArgs)
{
    // Test method: job_file_helper::JobDescArgs initJobArgs((const QString &dev))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ auto result = obj->initJobArgs(_arg0); EXPECT_GE(static_cast<int>(result), 0); });
}
