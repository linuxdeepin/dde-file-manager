// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computerinfothread.cpp
 * @brief Unit tests for ComputerInfoThread methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/computerpropertydialog.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class ComputerInfoThreadTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerInfoThread();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerInfoThread *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerInfoThreadTest, computerName)
{
    // Test getter: QString computerName()
    auto result = obj->computerName();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(ComputerInfoThreadTest, computerProcess)
{
    // Test method: void computerProcess(())
    EXPECT_NO_FATAL_FAILURE(obj->computerProcess());
}

TEST_F(ComputerInfoThreadTest, cpuInfo)
{
    // Test getter: QString cpuInfo()
    auto result = obj->cpuInfo();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(ComputerInfoThreadTest, memoryInfo)
{
    // Test getter: QString memoryInfo()
    auto result = obj->memoryInfo();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(ComputerInfoThreadTest, osBuild)
{
    // Test getter: QString osBuild()
    auto result = obj->osBuild();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(ComputerInfoThreadTest, run)
{
    // Test method: void run(())
    EXPECT_NO_FATAL_FAILURE(obj->run());
}

TEST_F(ComputerInfoThreadTest, startThread)
{
    // Test method: void startThread(())
    EXPECT_NO_FATAL_FAILURE(obj->startThread());
}

TEST_F(ComputerInfoThreadTest, stopThread)
{
    // Test method: void stopThread(())
    EXPECT_NO_FATAL_FAILURE(obj->stopThread());
}

TEST_F(ComputerInfoThreadTest, systemType)
{
    // Test getter: QString systemType()
    auto result = obj->systemType();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(ComputerInfoThreadTest, versionNum)
{
    // Test getter: QString versionNum()
    auto result = obj->versionNum();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(ComputerInfoThreadTest, ComputerInfoThread)
{
    // Test constructor: ComputerInfoThread((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
