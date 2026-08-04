// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IDLEMONITOR_H
#define IDLEMONITOR_H

#include "service_textindex_global.h"

#include <QObject>

SERVICETEXTINDEX_BEGIN_NAMESPACE

/**
 * @brief Event-driven idle detection using KIdleTime.
 *
 * Uses addIdleTimeout() to get notified when the idle threshold (default 30 s)
 * is reached, then catchNextResumeEvent() to detect when the user resumes
 * activity.  No polling — purely signal-driven per the KIdleTime design.
 *
 * KIdleTime is a hard build dependency (no HAVE_KIDLETIME guard): on Debian
 * the packaging always pulls libkf6idletime-dev, so the compile guard adds
 * complexity with no benefit.
 */
class IdleMonitor : public QObject
{
    Q_OBJECT
public:
    explicit IdleMonitor(QObject *parent = nullptr);
    ~IdleMonitor() override;

    void start();
    void stop();

    bool idle() const { return m_idle; }

Q_SIGNALS:
    void idleChanged(bool idle);

private Q_SLOTS:
    void onTimeoutReached(int identifier, int timeout);
    void onResumingFromIdle();

private:
    void setIdle(bool idle);

    bool m_idle { false };
    bool m_started { false };
    int m_idleThresholdMs { 30000 };
    int m_idleIdentifier { 0 };
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // IDLEMONITOR_H
