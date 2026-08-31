// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_dminitencryptworker.cpp
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

TEST_F(DMInitEncryptWorkerTest, detachPhyDevice)
{
    // Test method: int detachPhyDevice((int argc, const char *argv[]))
    auto result = obj->detachPhyDevice(0, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(DMInitEncryptWorkerTest, initJobArgs)
{
    // Test method: job_file_helper::JobDescArgs initJobArgs((const QString &phyDev, const QString &unlockName))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->initJobArgs(_arg0, _arg1);
    EXPECT_GE(static_cast<int>(result), 0);

}
