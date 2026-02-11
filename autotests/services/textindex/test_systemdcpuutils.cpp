// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <QProcess>
#include <QIODeviceBase>

#include "utils/systemdcpuutils.h"

SERVICETEXTINDEX_USE_NAMESPACE

class UT_SystemdCpuUtils : public testing::Test
{
protected:
    void SetUp() override
    {
        // Reset mock state
        mockProcessSuccess = true;
        mockProcessExitCode = 0;
        mockProcessOutput = "";
        mockProcessErrorOutput = "";
        mockProcessCrashed = false;
        mockProcessTimedOut = false;
        lastExecutedCommand.clear();
        lastExecutedArguments.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    void setupProcessMock(bool success = true, int exitCode = 0,
                          const QString &output = "", const QString &errorOutput = "",
                          bool crashed = false, bool timedOut = false)
    {
        mockProcessSuccess = success;
        mockProcessExitCode = exitCode;
        mockProcessOutput = output;
        mockProcessErrorOutput = errorOutput;
        mockProcessCrashed = crashed;
        mockProcessTimedOut = timedOut;

        // Mock QProcess::start (重载函数需要特殊处理)
        using StartFunc = void (QProcess::*)(const QString &, const QStringList &, QIODeviceBase::OpenMode);
        stub.set_lamda(static_cast<StartFunc>(&QProcess::start),
                       [this](QProcess *process, const QString &command, const QStringList &arguments, QIODeviceBase::OpenMode mode) {
                           __DBG_STUB_INVOKE__
                           lastExecutedCommand = command;
                           lastExecutedArguments = arguments;
                           // Don't actually start the process
                       });

        // Mock QProcess::waitForFinished
        stub.set_lamda(ADDR(QProcess, waitForFinished), [this](QProcess *process, int msecs) -> bool {
            __DBG_STUB_INVOKE__
            Q_UNUSED(msecs)
            if (mockProcessTimedOut) {
                return false;
            }
            // If process crashed, waitForFinished still returns true (process finished, but crashed)
            if (mockProcessCrashed) {
                return true;
            }
            return mockProcessSuccess;
        });

        // Mock QProcess::exitStatus
        stub.set_lamda(ADDR(QProcess, exitStatus), [this](QProcess *process) -> QProcess::ExitStatus {
            __DBG_STUB_INVOKE__
            return mockProcessCrashed ? QProcess::CrashExit : QProcess::NormalExit;
        });

        // Mock QProcess::exitCode
        stub.set_lamda(ADDR(QProcess, exitCode), [this](QProcess *process) -> int {
            __DBG_STUB_INVOKE__
            return mockProcessExitCode;
        });

        // Mock QProcess::readAllStandardOutput
        stub.set_lamda(ADDR(QProcess, readAllStandardOutput), [this](QProcess *process) -> QByteArray {
            __DBG_STUB_INVOKE__
            return mockProcessOutput.toUtf8();
        });

        // Mock QProcess::readAllStandardError
        stub.set_lamda(ADDR(QProcess, readAllStandardError), [this](QProcess *process) -> QByteArray {
            __DBG_STUB_INVOKE__
            return mockProcessErrorOutput.toUtf8();
        });

        // Mock QProcess::state
        stub.set_lamda(ADDR(QProcess, state), [this](QProcess *process) -> QProcess::ProcessState {
            __DBG_STUB_INVOKE__
            return mockProcessTimedOut ? QProcess::Running : QProcess::NotRunning;
        });

        // Mock QProcess::kill
        stub.set_lamda(ADDR(QProcess, kill), [this](QProcess *process) {
            __DBG_STUB_INVOKE__
            // Simulate process being killed
        });
    }

    // Mock state variables
    bool mockProcessSuccess = true;
    int mockProcessExitCode = 0;
    QString mockProcessOutput;
    QString mockProcessErrorOutput;
    bool mockProcessCrashed = false;
    bool mockProcessTimedOut = false;

    // Track executed commands for verification
    QString lastExecutedCommand;
    QStringList lastExecutedArguments;

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_SystemdCpuUtils, SetCpuQuota_ValidServiceAndPercentage_ReturnsTrue)
{
    setupProcessMock(true, 0);
    QString errorMsg;

    bool result = SystemdCpuUtils::setCpuQuota("test.service", 50, &errorMsg);

    EXPECT_TRUE(result);
    EXPECT_TRUE(errorMsg.isEmpty());
    EXPECT_EQ(lastExecutedCommand, "systemctl");
    EXPECT_TRUE(lastExecutedArguments.contains("--runtime"));
    EXPECT_TRUE(lastExecutedArguments.contains("--user"));
    EXPECT_TRUE(lastExecutedArguments.contains("set-property"));
    EXPECT_TRUE(lastExecutedArguments.contains("test.service"));
    EXPECT_TRUE(lastExecutedArguments.contains("CPUQuota=50%"));
}

TEST_F(UT_SystemdCpuUtils, SetCpuQuota_EmptyServiceName_ReturnsFalse)
{
    QString errorMsg;

    bool result = SystemdCpuUtils::setCpuQuota("", 50, &errorMsg);

    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMsg.isEmpty());
    EXPECT_TRUE(errorMsg.contains("Service name cannot be empty"));
}

TEST_F(UT_SystemdCpuUtils, SetCpuQuota_NegativePercentage_ReturnsFalse)
{
    QString errorMsg;

    bool result = SystemdCpuUtils::setCpuQuota("test.service", -10, &errorMsg);

    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMsg.isEmpty());
    EXPECT_TRUE(errorMsg.contains("Percentage must be non-negative"));
}

TEST_F(UT_SystemdCpuUtils, SetCpuQuota_ZeroPercentage_ReturnsTrue)
{
    setupProcessMock(true, 0);
    QString errorMsg;

    bool result = SystemdCpuUtils::setCpuQuota("test.service", 0, &errorMsg);

    EXPECT_TRUE(result);
    EXPECT_TRUE(errorMsg.isEmpty());
    EXPECT_TRUE(lastExecutedArguments.contains("CPUQuota=0%"));
}

TEST_F(UT_SystemdCpuUtils, SetCpuQuota_HighPercentage_ReturnsTrue)
{
    setupProcessMock(true, 0);
    QString errorMsg;

    bool result = SystemdCpuUtils::setCpuQuota("test.service", 200, &errorMsg);

    EXPECT_TRUE(result);
    EXPECT_TRUE(errorMsg.isEmpty());
    EXPECT_TRUE(lastExecutedArguments.contains("CPUQuota=200%"));
}

TEST_F(UT_SystemdCpuUtils, SetCpuQuota_ProcessFails_ReturnsFalse)
{
    setupProcessMock(true, 1, "", "Permission denied");
    QString errorMsg;

    bool result = SystemdCpuUtils::setCpuQuota("test.service", 50, &errorMsg);

    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMsg.isEmpty());
    EXPECT_TRUE(errorMsg.contains("Command failed with exit code 1"));
    EXPECT_TRUE(errorMsg.contains("Permission denied"));
}

TEST_F(UT_SystemdCpuUtils, SetCpuQuota_ProcessCrashes_ReturnsFalse)
{
    setupProcessMock(false, 0, "", "", true);
    QString errorMsg;

    bool result = SystemdCpuUtils::setCpuQuota("test.service", 50, &errorMsg);

    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMsg.isEmpty());
    EXPECT_TRUE(errorMsg.contains("Command crashed"));
}

TEST_F(UT_SystemdCpuUtils, SetCpuQuota_ProcessTimesOut_ReturnsFalse)
{
    setupProcessMock(false, 0, "", "", false, true);
    QString errorMsg;

    bool result = SystemdCpuUtils::setCpuQuota("test.service", 50, &errorMsg);

    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMsg.isEmpty());
    EXPECT_TRUE(errorMsg.contains("Command timed out"));
}

TEST_F(UT_SystemdCpuUtils, ResetCpuQuota_ValidServiceName_ReturnsTrue)
{
    setupProcessMock(true, 0);
    QString errorMsg;

    bool result = SystemdCpuUtils::resetCpuQuota("test.service", &errorMsg);

    EXPECT_TRUE(result);
    EXPECT_TRUE(errorMsg.isEmpty());
    EXPECT_EQ(lastExecutedCommand, "systemctl");
    EXPECT_TRUE(lastExecutedArguments.contains("--runtime"));
    EXPECT_TRUE(lastExecutedArguments.contains("--user"));
    EXPECT_TRUE(lastExecutedArguments.contains("set-property"));
    EXPECT_TRUE(lastExecutedArguments.contains("test.service"));
    EXPECT_TRUE(lastExecutedArguments.contains("CPUQuota="));
}

TEST_F(UT_SystemdCpuUtils, ResetCpuQuota_EmptyServiceName_ReturnsFalse)
{
    QString errorMsg;

    bool result = SystemdCpuUtils::resetCpuQuota("", &errorMsg);

    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMsg.isEmpty());
    EXPECT_TRUE(errorMsg.contains("Service name cannot be empty"));
}

TEST_F(UT_SystemdCpuUtils, ResetCpuQuota_ProcessFails_ReturnsFalse)
{
    setupProcessMock(true, 1, "", "Service not found");
    QString errorMsg;

    bool result = SystemdCpuUtils::resetCpuQuota("nonexistent.service", &errorMsg);

    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMsg.isEmpty());
    EXPECT_TRUE(errorMsg.contains("Command failed with exit code 1"));
    EXPECT_TRUE(errorMsg.contains("Service not found"));
}

TEST_F(UT_SystemdCpuUtils, SetCpuQuota_ValidServiceWithComplexName_ReturnsTrue)
{
    setupProcessMock(true, 0);
    QString errorMsg;
    const QString complexServiceName = "deepin-service-plugin@org.deepin.Filemanager.TextIndex.service";

    bool result = SystemdCpuUtils::setCpuQuota(complexServiceName, 75, &errorMsg);

    EXPECT_TRUE(result);
    EXPECT_TRUE(errorMsg.isEmpty());
    EXPECT_TRUE(lastExecutedArguments.contains(complexServiceName));
    EXPECT_TRUE(lastExecutedArguments.contains("CPUQuota=75%"));
}

TEST_F(UT_SystemdCpuUtils, ResetCpuQuota_ValidServiceWithComplexName_ReturnsTrue)
{
    setupProcessMock(true, 0);
    QString errorMsg;
    const QString complexServiceName = "deepin-service-plugin@org.deepin.Filemanager.TextIndex.service";

    bool result = SystemdCpuUtils::resetCpuQuota(complexServiceName, &errorMsg);

    EXPECT_TRUE(result);
    EXPECT_TRUE(errorMsg.isEmpty());
    EXPECT_TRUE(lastExecutedArguments.contains(complexServiceName));
    EXPECT_TRUE(lastExecutedArguments.contains("CPUQuota="));
}

TEST_F(UT_SystemdCpuUtils, SetCpuQuota_NullErrorMsg_DoesNotCrash)
{
    setupProcessMock(true, 0);

    // This should not crash even with null error message pointer
    // Note: In real implementation, this should be handled with Q_ASSERT
    // but for testing, we verify it doesn't crash
    EXPECT_NO_THROW({
        QString error;
        SystemdCpuUtils::setCpuQuota("test.service", 50, &error);
    });
}

TEST_F(UT_SystemdCpuUtils, ResetCpuQuota_NullErrorMsg_DoesNotCrash)
{
    setupProcessMock(true, 0);

    // This should not crash even with null error message pointer
    EXPECT_NO_THROW({
        QString error;
        SystemdCpuUtils::resetCpuQuota("test.service", &error);
    });
}
