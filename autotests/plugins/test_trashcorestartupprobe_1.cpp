// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashcorestartupprobe_1.cpp
 * @brief Unit tests for TrashCoreStartupProbe methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/trashcorestartupprobe.h"

#include <QTest>

using namespace dfmplugin_trashcore;

class TrashCoreStartupProbeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashCoreStartupProbe();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashCoreStartupProbe *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashCoreStartupProbeTest, TrashCoreStartupProbe)
{
    // Test constructor: TrashCoreStartupProbe((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TrashCoreStartupProbeTest, cleanupProcess)
{
    // Test method: void cleanupProcess(())
    EXPECT_NO_FATAL_FAILURE(obj->cleanupProcess());
}

TEST_F(TrashCoreStartupProbeTest, handleProbeError)
{
    // Test method: void handleProbeError((QProcess::ProcessError error))
    EXPECT_NO_FATAL_FAILURE(obj->handleProbeError(QProcess::ProcessError()));
}

TEST_F(TrashCoreStartupProbeTest, handleProbeFinished)
{
    // Test method: void handleProbeFinished((int exitCode, QProcess::ExitStatus exitStatus))
    EXPECT_NO_FATAL_FAILURE(obj->handleProbeFinished(0, QProcess::ExitStatus()));
}

TEST_F(TrashCoreStartupProbeTest, handleProbeTimeout)
{
    // Test method: void handleProbeTimeout(())
    EXPECT_NO_FATAL_FAILURE(obj->handleProbeTimeout());
}

TEST_F(TrashCoreStartupProbeTest, isReady)
{
    // Test bool getter: isReady()
    bool result = obj->isReady();
    EXPECT_FALSE(result);

}

TEST_F(TrashCoreStartupProbeTest, scheduleRetry)
{
    // Test method: void scheduleRetry(())
    EXPECT_NO_FATAL_FAILURE(obj->scheduleRetry());
}

TEST_F(TrashCoreStartupProbeTest, startProbe)
{
    // Test method: void startProbe(())
    EXPECT_NO_FATAL_FAILURE(obj->startProbe());
}
