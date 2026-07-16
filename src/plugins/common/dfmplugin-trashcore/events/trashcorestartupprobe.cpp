// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcorestartupprobe.h"

namespace dfmplugin_trashcore {

namespace {
constexpr int kProbeTimeoutMs = 120 * 1000;
constexpr int kProbeRetryIntervalMs = 60 * 1000;
}

TrashCoreStartupProbe::TrashCoreStartupProbe(QObject *parent)
    : QObject(parent)
{
    probeTimeoutTimer.setSingleShot(true);
    retryTimer.setSingleShot(true);

    connect(&probeTimeoutTimer, &QTimer::timeout,
            this, &TrashCoreStartupProbe::handleProbeTimeout);
    connect(&retryTimer, &QTimer::timeout,
            this, &TrashCoreStartupProbe::startProbe);
}

void TrashCoreStartupProbe::start()
{
    if (readyState || probeProcess)
        return;

    startProbe();
}

bool TrashCoreStartupProbe::isReady() const
{
    return readyState;
}

void TrashCoreStartupProbe::startProbe()
{
    if (readyState || probeProcess)
        return;

    probeProcess = new QProcess(this);
    probeProcess->setProgram("gio");
    probeProcess->setArguments({ "info", "trash:///" });

    connect(probeProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &TrashCoreStartupProbe::handleProbeFinished);
    connect(probeProcess, &QProcess::errorOccurred,
            this, &TrashCoreStartupProbe::handleProbeError);

    probeProcess->start();
    probeTimeoutTimer.start(kProbeTimeoutMs);
}

void TrashCoreStartupProbe::handleProbeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    probeTimeoutTimer.stop();

    const bool success = (exitStatus == QProcess::NormalExit && exitCode == 0);
    cleanupProcess();
    if (!success) {
        scheduleRetry();
        return;
    }

    readyState = true;
    emit ready();
}

void TrashCoreStartupProbe::handleProbeError(QProcess::ProcessError error)
{
    if (error != QProcess::FailedToStart)
        return;

    probeTimeoutTimer.stop();
    cleanupProcess();
    scheduleRetry();
}

void TrashCoreStartupProbe::handleProbeTimeout()
{
    if (!probeProcess)
        return;

    probeProcess->kill();
}

void TrashCoreStartupProbe::cleanupProcess()
{
    if (!probeProcess)
        return;

    probeProcess->deleteLater();
    probeProcess = nullptr;
}

void TrashCoreStartupProbe::scheduleRetry()
{
    if (readyState || retryTimer.isActive())
        return;

    retryTimer.start(kProbeRetryIntervalMs);
}

}   // namespace dfmplugin_trashcore
