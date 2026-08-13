// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_envdetector.cpp
 * @brief Unit tests for EnvDetector, PowerMonitor, and EnvState
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>

#include "stubext.h"

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/env/envdetector.h"
#include "services/textindex/env/powermonitor.h"
#include "services/textindex/env/idlemonitor.h"
#include "services/textindex/env/loadmonitor.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

// ---- EnvState tests ----

TEST(EnvStateTest, DefaultValues)
{
    EnvState state;
    EXPECT_FALSE(state.onBattery);
    EXPECT_FALSE(state.powerSaveMode);
    EXPECT_FALSE(state.idle);
}

TEST(EnvStateTest, IsPowerOk)
{
    EnvState state;
    EXPECT_TRUE(state.isPowerOk());
    state.onBattery = true;
    EXPECT_FALSE(state.isPowerOk());
}

TEST(EnvStateTest, IsPowerSaveOff)
{
    EnvState state;
    EXPECT_TRUE(state.isPowerSaveOff());
    state.powerSaveMode = true;
    EXPECT_FALSE(state.isPowerSaveOff());
}

TEST(EnvStateTest, IsIdleOk)
{
    EnvState state;
    EXPECT_FALSE(state.isIdleOk());
    state.idle = true;
    EXPECT_TRUE(state.isIdleOk());
}

// ---- PowerMonitor tests ----

class PowerMonitorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        if (!qApp)
            app = std::make_unique<QCoreApplication>(argc, argv);
    }

    std::unique_ptr<QCoreApplication> app;
    static int argc;
    static char *argv[];
};

int PowerMonitorTest::argc = 1;
char *PowerMonitorTest::argv[] = { const_cast<char *>("test_powermonitor") };

TEST_F(PowerMonitorTest, CtorDefaultValues)
{
    PowerMonitor pm;
    EXPECT_FALSE(pm.onBattery());
    EXPECT_FALSE(pm.powerSaveMode());
}

TEST_F(PowerMonitorTest, StartStop_NoCrash)
{
    PowerMonitor pm;
    pm.start();
    pm.stop();
    SUCCEED();
}

TEST_F(PowerMonitorTest, StartTwice)
{
    PowerMonitor pm;
    pm.start();
    pm.start();
    pm.stop();
    SUCCEED();
}

TEST_F(PowerMonitorTest, StopTwice)
{
    PowerMonitor pm;
    pm.stop();
    pm.stop();
    SUCCEED();
}

TEST_F(PowerMonitorTest, OnBatteryChanged_EnterBattery)
{
    PowerMonitor pm;
    QSignalSpy spy(&pm, &PowerMonitor::effectivePowerChanged);

    pm.onBatteryChanged(true);
    // Battery entry is delayed by 500ms timer, so no signal yet
    // But pending state should be set
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(PowerMonitorTest, OnBatteryChanged_NoChange)
{
    PowerMonitor pm;
    pm.m_pendingOnBattery = true;

    QSignalSpy spy(&pm, &PowerMonitor::effectivePowerChanged);
    pm.onBatteryChanged(true);
    EXPECT_EQ(spy.count(), 0);  // same value, no emit
}

TEST_F(PowerMonitorTest, OnBatteryChanged_ExitBattery)
{
    PowerMonitor pm;
    pm.m_onBattery = true;
    pm.m_pendingOnBattery = true;

    QSignalSpy spy(&pm, &PowerMonitor::effectivePowerChanged);
    pm.onBatteryChanged(false);
    // Exiting battery is immediate (timer stopped, direct assignment)
    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(pm.onBattery());
    EXPECT_FALSE(spy.at(0).at(0).toBool());
}

TEST_F(PowerMonitorTest, OnModeChanged_EnterPowerSave)
{
    PowerMonitor pm;
    QSignalSpy spy(&pm, &PowerMonitor::effectivePowerChanged);
    pm.onModeChanged("powersave");
    // Entry is delayed by 500ms
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(PowerMonitorTest, OnModeChanged_NoChange)
{
    PowerMonitor pm;
    pm.m_pendingPowerSave = true;
    pm.onModeChanged("powersave");
    SUCCEED();
}

TEST_F(PowerMonitorTest, OnModeChanged_ExitPowerSave)
{
    PowerMonitor pm;
    pm.m_powerSave = true;
    pm.m_pendingPowerSave = true;

    QSignalSpy spy(&pm, &PowerMonitor::effectivePowerChanged);
    pm.onModeChanged("performance");
    // Exit is delayed by 500ms
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(PowerMonitorTest, OnPropertiesChanged_Battery)
{
    PowerMonitor pm;
    QSignalSpy spy(&pm, &PowerMonitor::effectivePowerChanged);
    QVariantMap changed;
    changed["OnBattery"] = true;
    pm.onPropertiesChanged("org.freedesktop.DBus.Properties", changed, {});
    SUCCEED();
}

TEST_F(PowerMonitorTest, OnPropertiesChanged_Mode)
{
    PowerMonitor pm;
    QVariantMap changed;
    changed["Mode"] = "powersave";
    pm.onPropertiesChanged("org.freedesktop.DBus.Properties", changed, {});
    SUCCEED();
}

TEST_F(PowerMonitorTest, OnPropertiesChanged_Unrelated)
{
    PowerMonitor pm;
    QVariantMap changed;
    changed["SomeOtherProperty"] = 42;
    pm.onPropertiesChanged("org.freedesktop.DBus.Properties", changed, {});
    // No battery/mode change -> no action
    SUCCEED();
}

// ---- EnvDetector tests ----

class EnvDetectorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        if (!qApp)
            app = std::make_unique<QCoreApplication>(argc, argv);
    }

    std::unique_ptr<QCoreApplication> app;
    static int argc;
    static char *argv[];
};

int EnvDetectorTest::argc = 1;
char *EnvDetectorTest::argv[] = { const_cast<char *>("test_envdetector") };

TEST_F(EnvDetectorTest, CtorDefaultState)
{
    EnvDetector detector;
    EnvState state = detector.currentState();
    EXPECT_FALSE(state.onBattery);
    EXPECT_FALSE(state.powerSaveMode);
}

TEST_F(EnvDetectorTest, StartStop)
{
    EnvDetector detector;
    detector.start();
    detector.stop();
    SUCCEED();
}

TEST_F(EnvDetectorTest, StartTwice)
{
    EnvDetector detector;
    detector.start();
    detector.start();
    detector.stop();
    SUCCEED();
}

TEST_F(EnvDetectorTest, StopTwice)
{
    EnvDetector detector;
    detector.stop();
    detector.stop();
    SUCCEED();
}

TEST_F(EnvDetectorTest, SetDataPath)
{
    EnvDetector detector;
    detector.setDataPath("/tmp");
    SUCCEED();
}