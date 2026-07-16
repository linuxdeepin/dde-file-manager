// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHCORESTARTUPPROBE_H
#define TRASHCORESTARTUPPROBE_H

#include "dfmplugin_trashcore_global.h"

#include <QObject>
#include <QProcess>
#include <QTimer>

namespace dfmplugin_trashcore {

class TrashCoreStartupProbe final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TrashCoreStartupProbe)

public:
    explicit TrashCoreStartupProbe(QObject *parent = nullptr);

    void start();
    bool isReady() const;

Q_SIGNALS:
    void ready();

private Q_SLOTS:
    void startProbe();
    void handleProbeFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleProbeError(QProcess::ProcessError error);
    void handleProbeTimeout();

private:
    void cleanupProcess();
    void scheduleRetry();

private:
    QProcess *probeProcess { nullptr };
    QTimer probeTimeoutTimer;
    QTimer retryTimer;
    bool readyState { false };
};

}   // namespace dfmplugin_trashcore

#endif   // TRASHCORESTARTUPPROBE_H
