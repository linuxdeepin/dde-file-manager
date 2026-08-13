// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_idlemonitor.cpp
 * @brief Additional unit tests for IdleMonitor (env/idlemonitor.cpp)
 *        Appended to improve coverage of onTimeoutReached, onResumingFromIdle,
 *        stop-when-not-started, start-twice, and idle() getter.
 */

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QCoreApplication>

#include "stubext.h"

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/env/idlemonitor.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class IdleMonitorExtraTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        if (!qApp)
            app = std::make_unique<QCoreApplication>(argc, argv);
        monitor = std::make_unique<IdleMonitor>();
    }

    void TearDown() override
    {
        monitor.reset();
    }

    std::unique_ptr<QCoreApplication> app;
    std::unique_ptr<IdleMonitor> monitor;

    static int argc;
    static char *argv[];
};

int IdleMonitorExtraTest::argc = 1;
char *IdleMonitorExtraTest::argv[] = { const_cast<char *>("test_idlemonitor_extra") };

TEST_F(IdleMonitorExtraTest, IdleDefaultFalse)
{
    EXPECT_FALSE(monitor->idle());
}

TEST_F(IdleMonitorExtraTest, StopWhenNotStarted_NoCrash)
{
    // stop() checks m_started; when false, returns immediately
    EXPECT_NO_FATAL_FAILURE({ monitor->stop(); });
    EXPECT_FALSE(monitor->idle());
}

TEST_F(IdleMonitorExtraTest, StartTwice_NoCrash)
{
    monitor->start();
    monitor->start();
    // Second call returns early because m_started is already true
    monitor->stop();
    SUCCEED();
}

TEST_F(IdleMonitorExtraTest, StopTwice_NoCrash)
{
    monitor->start();
    monitor->stop();
    monitor->stop();
    EXPECT_FALSE(monitor->idle());
}

TEST_F(IdleMonitorExtraTest, OnTimeoutReached_SetsIdle)
{
    QSignalSpy spy(monitor.get(), &IdleMonitor::idleChanged);
    // Force started so the slot does real work
    monitor->m_started = true;

    monitor->onTimeoutReached(1, 30000);

    EXPECT_TRUE(monitor->idle());
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(spy.at(0).at(0).toBool());

    monitor->stop();
}

TEST_F(IdleMonitorExtraTest, OnTimeoutReached_WhenAlreadyIdle_NoExtraSignal)
{
    QSignalSpy spy(monitor.get(), &IdleMonitor::idleChanged);
    monitor->m_started = true;
    monitor->m_idle = true;

    monitor->onTimeoutReached(1, 30000);

    // Already idle, setIdle not called -> no signal
    EXPECT_EQ(spy.count(), 0);

    monitor->stop();
}

TEST_F(IdleMonitorExtraTest, OnResumingFromIdle_ClearsIdle)
{
    QSignalSpy spy(monitor.get(), &IdleMonitor::idleChanged);
    monitor->m_started = true;
    monitor->m_idle = true;

    monitor->onResumingFromIdle();

    EXPECT_FALSE(monitor->idle());
    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(spy.at(0).at(0).toBool());

    monitor->stop();
}

TEST_F(IdleMonitorExtraTest, OnResumingFromIdle_WhenNotIdle_NoSignal)
{
    QSignalSpy spy(monitor.get(), &IdleMonitor::idleChanged);
    monitor->m_idle = false;

    monitor->onResumingFromIdle();

    EXPECT_EQ(spy.count(), 0);
}

TEST_F(IdleMonitorExtraTest, DestroyWhileStarted_NoCrash)
{
    // Let the unique_ptr destructor call ~IdleMonitor while started
    monitor->start();
    monitor.reset();
    SUCCEED();
}