// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fstabdecryptworker.cpp
 * @brief Unit tests for FstabDecryptWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/workers/fstabdecryptworker.h"

#include <QTest>

using namespace src;

class FstabDecryptWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FstabDecryptWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FstabDecryptWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FstabDecryptWorkerTest, clearDeviceUUID)
{
    // Test method: QString clearDeviceUUID((const QString &dev))
    QString _arg0{};
    auto result = obj->clearDeviceUUID(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
