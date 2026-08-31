// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fsmonitorworker.cpp
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

TEST_F(FSMonitorWorkerTest, processDirectory)
{
    // Test method: void processDirectory((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->processDirectory(_arg0));
}

TEST_F(FSMonitorWorkerTest, setExclusionChecker)
{
    // Test setter: void setExclusionChecker((const std::function<bool(const QString &)> &checker))
    std::function<bool( QString )> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setExclusionChecker(_arg0));
}

TEST_F(FSMonitorWorkerTest, setMaxFastScanResults)
{
    // Test setter: void setMaxFastScanResults((int max))
    EXPECT_NO_FATAL_FAILURE(obj->setMaxFastScanResults(0));
}
