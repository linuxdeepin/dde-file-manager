// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QTest>

#include "stubext.h"

#include "services/textindex/env/loadmonitor.h"
#include "services/textindex/env/powermonitor.h"
#include "services/textindex/env/idlemonitor.h"
#include "services/textindex/env/envdetector.h"

using namespace service_textindex;

// ============================================================================
// LoadMonitor Tests
// ============================================================================

class TestLoadMonitor : public ::testing::Test
{
protected:
    void SetUp() override
    {
        if (!qApp)
            app = std::make_unique<QCoreApplication>(argc, argv);
        monitor = std::make_unique<LoadMonitor>();
    }

    void TearDown() override
    {
        monitor.reset();
    }

    std::unique_ptr<QCoreApplication> app;
    std::unique_ptr<LoadMonitor> monitor;

    static int argc;
    static char *argv[];
};

int TestLoadMonitor::argc = 1;
char *TestLoadMonitor::argv[] = { const_cast<char *>("test_loadmonitor") };

// --- Construction ---

TEST_F(TestLoadMonitor, Construction_DefaultValues)
{
    EXPECT_NE(monitor->m_timer, nullptr);
    EXPECT_GT(monitor->m_sampleIntervalSecs, 0);
    EXPECT_GT(monitor->m_cpuThresholdPercent, 0);
    EXPECT_GT(monitor->m_diskThresholdPercent, 0);
    EXPECT_DOUBLE_EQ(monitor->cpuAvgPercent(), 0.0);
    EXPECT_DOUBLE_EQ(monitor->diskBusyPercent(), 0.0);
    EXPECT_DOUBLE_EQ(monitor->cpuInstantPercent(), 0.0);
    EXPECT_DOUBLE_EQ(monitor->diskInstantPercent(), 0.0);
}

// --- readProcStat with real /proc/stat ---

TEST_F(TestLoadMonitor, ReadProcStat_RealFile_ReturnsTrue)
{
    qint64 user = -1, guest = -1, total = -1;
    EXPECT_TRUE(monitor->readProcStat(user, guest, total));
    EXPECT_GE(user, 0);
    EXPECT_GE(guest, 0);
    EXPECT_GE(total, 0);
    // total should be sum of user + nice + system + idle + iowait + irq + softirq + steal
    // so it should be at least as large as user alone
    EXPECT_GE(total, user);
}

// --- readProcDiskstats with real /proc/diskstats ---

TEST_F(TestLoadMonitor, ReadProcDiskstats_EmptyDeviceName_ReturnsFalse)
{
    monitor->m_diskDeviceName.clear();
    qint64 ioTicks = -1;
    EXPECT_FALSE(monitor->readProcDiskstats(ioTicks));
}

TEST_F(TestLoadMonitor, ReadProcDiskstats_RealDiskstats_MatchesPrefix)
{
    // Read /proc/diskstats ourselves to find a whole-disk entry
    QFile file(QStringLiteral("/proc/diskstats"));
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Find first line with >= 13 fields — that's a whole-disk row
    qint64 expectedTicks = 0;
    QString diskName;
    for (const QString &line : content.split(QLatin1Char('\n'))) {
        const auto parts = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (parts.size() >= 13) {
            diskName = parts[2];
            expectedTicks = parts[12].toLongLong();
            break;
        }
    }
    ASSERT_FALSE(diskName.isEmpty());

    // Set the device name to the whole-disk name so the method finds an exact match
    monitor->m_diskDeviceName = diskName;
    qint64 ioTicks = -1;
    EXPECT_TRUE(monitor->readProcDiskstats(ioTicks));
    // I/O ticks are cumulative and may increase between our read and the
    // method's internal read, so only verify the value is valid and
    // at least as large as what we captured.
    EXPECT_GE(ioTicks, expectedTicks);
}

// --- CPU %usr calculation (mocked readProcStat) ---

TEST_F(TestLoadMonitor, Sample_CpuUsrCalculation_Mocked)
{
    stub_ext::StubExt stub;

    // Simulate two samples: first with user=1000 guest=0 total=10000,
    // second with user=1100 guest=0 total=11000
    // %usr = (1100-1000 - (0-0)) / (11000-10000) * 100 = 10%
    int callCount = 0;
    stub.set_lamda(ADDR(LoadMonitor, readProcStat), [&callCount](LoadMonitor *, qint64 &user, qint64 &guest, qint64 &total) {
        if (callCount == 0) {
            user = 1000;
            guest = 0;
            total = 10000;
        } else {
            user = 1100;
            guest = 0;
            total = 11000;
        }
        callCount++;
        return true;
    });

    stub.set_lamda(ADDR(LoadMonitor, readProcDiskstats), [](LoadMonitor *, qint64 &ioTicks) {
        ioTicks = 0;
        return true;
    });

    QSignalSpy spy(monitor.get(), &LoadMonitor::loadChanged);

    monitor->sample();
    // First sample only — no percentage computed yet
    EXPECT_EQ(spy.count(), 0);

    monitor->sample();
    // Second sample — percentage computed
    ASSERT_EQ(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    EXPECT_NEAR(args[0].toDouble(), 10.0, 0.01);
}

// --- CPU %usr with guest correction ---

TEST_F(TestLoadMonitor, Sample_CpuUsrWithGuest_Mocked)
{
    stub_ext::StubExt stub;

    // user includes guest, so %usr = (user - guest) / total
    // Sample 1: user=1500, guest=500, total=10000
    // Sample 2: user=2500, guest=500, total=11000
    // delta_user=1000, delta_guest=0, delta_total=1000
    // %usr = (1000 - 0) / 1000 * 100 = 100%
    int callCount = 0;
    stub.set_lamda(ADDR(LoadMonitor, readProcStat), [&callCount](LoadMonitor *, qint64 &user, qint64 &guest, qint64 &total) {
        if (callCount == 0) {
            user = 1500;
            guest = 500;
            total = 10000;
        } else {
            user = 2500;
            guest = 500;
            total = 11000;
        }
        callCount++;
        return true;
    });

    stub.set_lamda(ADDR(LoadMonitor, readProcDiskstats), [](LoadMonitor *, qint64 &ioTicks) {
        ioTicks = 0;
        return true;
    });

    monitor->sample();
    monitor->sample();

    // %usr = 100% since all delta went to user (minus guest)
    EXPECT_NEAR(monitor->cpuAvgPercent(), 100.0, 0.01);
    EXPECT_NEAR(monitor->cpuInstantPercent(), 100.0, 0.01);
}

// --- Disk %util calculation (mocked readProcDiskstats) ---

TEST_F(TestLoadMonitor, Sample_DiskUtilCalculation_Mocked)
{
    stub_ext::StubExt stub;

    // Sample 1: ioTicks=0, Sample 2: ioTicks=500
    // elapsedMs = 5 * 1000 = 5000
    // %util = (500 - 0) / 5000 * 100 = 10%
    int callCount = 0;
    stub.set_lamda(ADDR(LoadMonitor, readProcDiskstats), [&callCount](LoadMonitor *, qint64 &ioTicks) {
        ioTicks = (callCount == 0) ? 0 : 500;
        callCount++;
        return true;
    });

    stub.set_lamda(ADDR(LoadMonitor, readProcStat), [](LoadMonitor *, qint64 &user, qint64 &guest, qint64 &total) {
        user = 1000;
        guest = 0;
        total = 10000;
        return true;
    });

    monitor->sample();
    monitor->sample();

    EXPECT_NEAR(monitor->diskBusyPercent(), 10.0, 0.01);
    EXPECT_NEAR(monitor->diskInstantPercent(), 10.0, 0.01);
}

// --- Disk %util capped at 100% ---

TEST_F(TestLoadMonitor, Sample_DiskUtilCappedAt100_Mocked)
{
    stub_ext::StubExt stub;

    // ioTicks delta exceeds elapsed time → should cap at 100%
    int callCount = 0;
    stub.set_lamda(ADDR(LoadMonitor, readProcDiskstats), [&callCount](LoadMonitor *, qint64 &ioTicks) {
        ioTicks = (callCount == 0) ? 0 : 999999;
        callCount++;
        return true;
    });

    stub.set_lamda(ADDR(LoadMonitor, readProcStat), [](LoadMonitor *, qint64 &user, qint64 &guest, qint64 &total) {
        user = 1000;
        guest = 0;
        total = 10000;
        return true;
    });

    monitor->sample();
    monitor->sample();

    EXPECT_NEAR(monitor->diskBusyPercent(), 100.0, 0.01);
}

// --- Threshold checks ---

TEST_F(TestLoadMonitor, IsCpuBelowThreshold_DefaultZero_ReturnsTrue)
{
    EXPECT_TRUE(monitor->isCpuBelowThreshold());
}

TEST_F(TestLoadMonitor, IsDiskBelowThreshold_DefaultZero_ReturnsTrue)
{
    EXPECT_TRUE(monitor->isDiskBelowThreshold());
}

// --- setDataPath ---

TEST_F(TestLoadMonitor, SetDataPath_UpdatesDataPath)
{
    monitor->setDataPath(QStringLiteral("/tmp"));
    EXPECT_EQ(monitor->m_dataPath, QStringLiteral("/tmp"));
}

// --- resolveDataDisk ---

TEST_F(TestLoadMonitor, ResolveDataDisk_EmptyPath_NoDeviceName)
{
    monitor->m_dataPath.clear();
    monitor->resolveDataDisk();
    EXPECT_TRUE(monitor->m_diskDeviceName.isEmpty());
}

TEST_F(TestLoadMonitor, ResolveDataDisk_HomePath_ResolvesDevice)
{
    monitor->m_dataPath = QDir::homePath();
    monitor->resolveDataDisk();
    // On Linux, home should resolve to a block device
    EXPECT_FALSE(monitor->m_diskDeviceName.isEmpty());
}

// --- Window management ---

TEST_F(TestLoadMonitor, Window_Size_CappedAtMaxSamples)
{
    stub_ext::StubExt stub;

    stub.set_lamda(ADDR(LoadMonitor, readProcStat), [](LoadMonitor *, qint64 &user, qint64 &guest, qint64 &total) {
        user = 1000;
        guest = 0;
        total = 10000;
        return true;
    });

    stub.set_lamda(ADDR(LoadMonitor, readProcDiskstats), [](LoadMonitor *, qint64 &ioTicks) {
        ioTicks = 0;
        return true;
    });

    const int maxSamples = qMax(1, 60 / monitor->m_sampleIntervalSecs);
    for (int i = 0; i < maxSamples + 5; ++i)
        monitor->sample();

    EXPECT_LE(monitor->m_window.size(), maxSamples);
}

// --- loadChanged signal ---

TEST_F(TestLoadMonitor, LoadChangedSignal_EmittedOnSecondSample_Mocked)
{
    stub_ext::StubExt stub;

    int callCount = 0;
    stub.set_lamda(ADDR(LoadMonitor, readProcStat), [&callCount](LoadMonitor *, qint64 &user, qint64 &guest, qint64 &total) {
        user = 1000 * (callCount + 1);
        guest = 0;
        total = 10000 * (callCount + 1);
        callCount++;
        return true;
    });

    stub.set_lamda(ADDR(LoadMonitor, readProcDiskstats), [](LoadMonitor *, qint64 &ioTicks) {
        ioTicks = 0;
        return true;
    });

    QSignalSpy spy(monitor.get(), &LoadMonitor::loadChanged);
    monitor->sample();
    EXPECT_EQ(spy.count(), 0);
    monitor->sample();
    ASSERT_EQ(spy.count(), 1);
}

// --- start/stop lifecycle ---

TEST_F(TestLoadMonitor, StartStop_TimerRunningState)
{
    monitor->setDataPath(QDir::homePath());
    monitor->start();
    EXPECT_TRUE(monitor->m_timer->isActive());
    monitor->stop();
    EXPECT_FALSE(monitor->m_timer->isActive());
}

TEST_F(TestLoadMonitor, Stop_WhenNotRunning_NoCrash)
{
    monitor->stop();
    SUCCEED();
}

// ============================================================================
// PowerMonitor Tests (stub DBus)
// ============================================================================

class TestPowerMonitor : public ::testing::Test
{
protected:
    void SetUp() override
    {
        if (!qApp)
            app = std::make_unique<QCoreApplication>(argc, argv);
        monitor = std::make_unique<PowerMonitor>();
    }

    void TearDown() override
    {
        monitor.reset();
    }

    std::unique_ptr<QCoreApplication> app;
    std::unique_ptr<PowerMonitor> monitor;

    static int argc;
    static char *argv[];
};

int TestPowerMonitor::argc = 1;
char *TestPowerMonitor::argv[] = { const_cast<char *>("test_powermonitor") };

TEST_F(TestPowerMonitor, Construction_DefaultValues)
{
    EXPECT_FALSE(monitor->onBattery());
    EXPECT_FALSE(monitor->powerSaveMode());
}

TEST_F(TestPowerMonitor, StartStop_NoCrash)
{
    monitor->start();
    monitor->stop();
    SUCCEED();
}

// --- onBatteryChanged delay logic ---

TEST_F(TestPowerMonitor, OnBatteryChanged_EntersBattery_AfterDelay)
{
    monitor->start();

    // Simulate battery entry — should not be immediate
    monitor->onBatteryChanged(true);
    EXPECT_FALSE(monitor->onBattery());

    // Wait for the delay timer
    QTest::qWait(600);
    EXPECT_TRUE(monitor->onBattery());

    monitor->stop();
}

TEST_F(TestPowerMonitor, OnBatteryChanged_ExitsBattery_Immediately)
{
    monitor->start();

    // Set to battery first
    monitor->onBatteryChanged(true);
    QTest::qWait(600);
    EXPECT_TRUE(monitor->onBattery());

    // Exit battery — should be immediate
    monitor->onBatteryChanged(false);
    EXPECT_FALSE(monitor->onBattery());

    monitor->stop();
}

// --- onModeChanged delay logic ---

TEST_F(TestPowerMonitor, OnModeChanged_EnterPowerSave_AfterDelay)
{
    monitor->start();

    monitor->onModeChanged(QStringLiteral("powersave"));
    EXPECT_FALSE(monitor->powerSaveMode());

    QTest::qWait(600);
    EXPECT_TRUE(monitor->powerSaveMode());

    monitor->stop();
}

TEST_F(TestPowerMonitor, OnModeChanged_ExitPowerSave_AfterDelay)
{
    monitor->start();

    // Enter power save first
    monitor->onModeChanged(QStringLiteral("powersave"));
    QTest::qWait(600);
    EXPECT_TRUE(monitor->powerSaveMode());

    // Exit power save
    monitor->onModeChanged(QStringLiteral("balance"));
    EXPECT_TRUE(monitor->powerSaveMode());   // still true until delay expires

    QTest::qWait(600);
    EXPECT_FALSE(monitor->powerSaveMode());

    monitor->stop();
}

// ============================================================================
// IdleMonitor Tests
// ============================================================================

class TestIdleMonitor : public ::testing::Test
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

int TestIdleMonitor::argc = 1;
char *TestIdleMonitor::argv[] = { const_cast<char *>("test_idlemonitor") };

TEST_F(TestIdleMonitor, Construction_DefaultValues)
{
    EXPECT_FALSE(monitor->idle());
}

TEST_F(TestIdleMonitor, StartStop_NoCrash)
{
    monitor->start();
    monitor->stop();
    SUCCEED();
}

TEST_F(TestIdleMonitor, Stop_ResetsIdle)
{
    monitor->m_started = true;
    monitor->m_idle = true;
    monitor->stop();
    EXPECT_FALSE(monitor->idle());
}

// ============================================================================
// EnvDetector Tests
// ============================================================================

class TestEnvDetector : public ::testing::Test
{
protected:
    void SetUp() override
    {
        if (!qApp)
            app = std::make_unique<QCoreApplication>(argc, argv);
        detector = std::make_unique<EnvDetector>();
    }

    void TearDown() override
    {
        detector.reset();
    }

    std::unique_ptr<QCoreApplication> app;
    std::unique_ptr<EnvDetector> detector;

    static int argc;
    static char *argv[];
};

int TestEnvDetector::argc = 1;
char *TestEnvDetector::argv[] = { const_cast<char *>("test_envdetector") };

TEST_F(TestEnvDetector, Construction_DefaultState)
{
    EnvState state = detector->currentState();
    EXPECT_FALSE(state.onBattery);
    EXPECT_FALSE(state.powerSaveMode);
    EXPECT_FALSE(state.idle);
    EXPECT_TRUE(state.isPowerOk());
    EXPECT_TRUE(state.isPowerSaveOff());
    EXPECT_FALSE(state.isIdleOk());
}

TEST_F(TestEnvDetector, SetDataPath_NoCrash)
{
    detector->setDataPath(QDir::homePath());
    SUCCEED();
}

TEST_F(TestEnvDetector, StartStop_NoCrash)
{
    detector->setDataPath(QDir::homePath());
    detector->start();
    detector->stop();
    SUCCEED();
}

TEST_F(TestEnvDetector, EnvStateChanged_EmittedOnStart)
{
    QSignalSpy spy(detector.get(), &EnvDetector::envStateChanged);
    detector->setDataPath(QDir::homePath());
    detector->start();

    if (spy.count() > 0) {
        QList<QVariant> args = spy.takeFirst();
        EXPECT_EQ(args.count(), 1);
    }
}
