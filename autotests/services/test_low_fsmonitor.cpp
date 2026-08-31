// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_fsmonitor.cpp
 * @brief Unit tests for FSMonitor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/fsmonitor/fsmonitor.h"

#include <QTest>

using namespace src;

class FSMonitorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FSMonitor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FSMonitor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FSMonitorTest, initialize)
{
    // Test method: bool initialize((const QStringList &rootPaths))
    QStringList _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}
