// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "systemdcpuutils.h"

#include <QProcess>
#include <QDebug>
#include <QStringList>
#include <atomic>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace {

bool executeSystemctlCommand(const QStringList &arguments, QString *errorMsg)
{
    QProcess process;
    QString command = "systemctl";

    // 打印将要执行的命令，方便调试
    fmDebug() << "SystemdCpuUtils: Executing command:" << command << arguments.join(" ");

    process.start(command, arguments);

    // 等待命令完成，设置一个超时时间（例如 5 秒）
    if (!process.waitForFinished(5000)) {
        *errorMsg = QString("Command timed out: %1 %2").arg(command, arguments.join(" "));
        if (process.state() == QProcess::Running) {
            process.kill();
            process.waitForFinished(1000);   // Give it a moment to die
        }
        return false;
    }

    if (process.exitStatus() == QProcess::CrashExit) {
        *errorMsg = QString("Command crashed: %1 %2").arg(command, arguments.join(" "));
        return false;
    }

    if (process.exitCode() != 0) {
        *errorMsg = QString("Command failed with exit code %1: %2 %3\nError: %4\nOutput: %5")
                            .arg(process.exitCode())
                            .arg(command)
                            .arg(arguments.join(" "))
                            .arg(QString::fromUtf8(process.readAllStandardError()))
                            .arg(QString::fromUtf8(process.readAllStandardOutput()));
        return false;
    }

    fmDebug() << "SystemdCpuUtils: Command executed successfully";
    const QString output = QString::fromUtf8(process.readAllStandardOutput());
    if (!output.isEmpty()) {
        fmDebug() << "SystemdCpuUtils: Command output:" << output;
    }
    return true;
}

}   // anonymous namespace

// 实现 SystemdCpuUtils 命名空间中的函数
namespace SystemdCpuUtils {

// 全局引用计数器：跟踪当前有多少个限制型任务(Light/Medium/Heavy)在运行。
// 只有计数从 0→1 时才实际 setCpuQuota，从 1→0 时才实际 resetCpuQuota。
// Manual 任务不改变计数器，仅在计数为 0 时才 reset。
// 当 Manual 启动时，同一 TaskManager 的当前任务已被 stopCurrentTask 停止，
// 其 releaseResourcePolicy 先执行使计数递减，因此 Manual 的 applyResourcePolicy
// 执行时计数已正确反映实际状态。
static std::atomic<int> g_limitedTaskCount { 0 };

bool setCpuQuota(const QString &serviceName, int percentage, QString *errorMsg)
{
    Q_ASSERT(errorMsg);

    if (serviceName.isEmpty()) {
        *errorMsg = "Service name cannot be empty.";
        return false;
    }
    if (percentage < 0) {   // systemd 允许 > 100%，但这里通常限制为非负
        *errorMsg = "Percentage must be non-negative.";
        return false;
    }

    QStringList arguments;
    arguments << "--runtime"
              << "--user"
              << "set-property"
              << serviceName
              << QString("CPUQuota=%1%").arg(percentage);

    return executeSystemctlCommand(arguments, errorMsg);   // 调用匿名命名空间中的辅助函数
}

bool resetCpuQuota(const QString &serviceName, QString *errorMsg)
{
    Q_ASSERT(errorMsg);

    if (serviceName.isEmpty()) {
        *errorMsg = "Service name cannot be empty.";
        return false;
    }

    QStringList arguments;
    arguments << "--runtime"
              << "--user"
              << "set-property"
              << serviceName
              << "CPUQuota=";   // 设置为空字符串以取消限制

    return executeSystemctlCommand(arguments, errorMsg);   // 调用匿名命名空间中的辅助函数
}

void acquireCpuQuota(const QString &serviceName, int percentage)
{
    int prev = g_limitedTaskCount.fetch_add(1, std::memory_order_acq_rel);
    if (prev == 0) {
        QString msg;
        if (!setCpuQuota(serviceName, percentage, &msg)) {
            fmWarning() << "SystemdCpuUtils: acquireCpuQuota failed to set CPU quota:" << msg;
        }
    }
}

void releaseCpuQuota(const QString &serviceName)
{
    int prev = g_limitedTaskCount.fetch_sub(1, std::memory_order_acq_rel);
    if (prev == 1) {
        QString msg;
        if (!resetCpuQuota(serviceName, &msg)) {
            fmWarning() << "SystemdCpuUtils: releaseCpuQuota failed to reset CPU quota:" << msg;
        }
    }
}

void manualTaskCpuQuota(const QString &serviceName)
{
    if (g_limitedTaskCount.load(std::memory_order_acquire) == 0) {
        QString msg;
        if (!resetCpuQuota(serviceName, &msg)) {
            fmWarning() << "SystemdCpuUtils: manualTaskCpuQuota failed to reset CPU quota:" << msg;
        }
    }
}

}   // namespace SystemdCpuUtils

SERVICETEXTINDEX_END_NAMESPACE
