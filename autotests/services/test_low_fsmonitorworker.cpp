// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_fsmonitorworker.cpp
 * @brief Unit tests for FSMonitorWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/fsmonitor/fsmonitorworker.h"

#include <QTest>

using namespace src;

class FSMonitorWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FSMonitorWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FSMonitorWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FSMonitorWorkerTest, fetchDirectoriesViaApi)
{
    // Test getter: QStringList fetchDirectoriesViaApi()
    auto result = obj->fetchDirectoriesViaApi();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FSMonitorWorkerTest, fetchDirectoriesViaCli)
{
    // Test getter: QStringList fetchDirectoriesViaCli()
    auto result = obj->fetchDirectoriesViaCli();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FSMonitorWorkerTest, handleFastScanResult)
{
    // Test method: void handleFastScanResult(())
    EXPECT_NO_FATAL_FAILURE(obj->handleFastScanResult());
}

TEST_F(FSMonitorWorkerTest, FSMonitorWorker_Destructor)
{
    // Test method:  ~FSMonitorWorker(())
    EXPECT_NO_FATAL_FAILURE({ FSMonitorWorker *tmp = new FSMonitorWorker(); delete tmp; });
}
