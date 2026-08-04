// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "idlemonitor.h"
#include "utils/textindexconfig.h"

#include <KIdleTime>

SERVICETEXTINDEX_BEGIN_NAMESPACE

IdleMonitor::IdleMonitor(QObject *parent)
    : QObject(parent)
{
    m_idleThresholdMs = TextIndexConfig::instance().idleThresholdSeconds() * 1000;

    auto *idle = KIdleTime::instance();
    connect(idle, &KIdleTime::timeoutReached, this, &IdleMonitor::onTimeoutReached);
    connect(idle, &KIdleTime::resumingFromIdle, this, &IdleMonitor::onResumingFromIdle);
}

IdleMonitor::~IdleMonitor() = default;

void IdleMonitor::start()
{
    if (m_started)
        return;
    m_started = true;

    m_idleIdentifier = KIdleTime::instance()->addIdleTimeout(m_idleThresholdMs);
}

void IdleMonitor::stop()
{
    if (!m_started)
        return;
    m_started = false;

    KIdleTime::instance()->removeIdleTimeout(m_idleIdentifier);
    setIdle(false);
}

void IdleMonitor::onTimeoutReached(int identifier, int timeout)
{
    Q_UNUSED(identifier)
    Q_UNUSED(timeout)

    if (!m_idle)
        setIdle(true);

    // After the idle threshold is reached, register for the next resume event
    // so we get notified the moment the user becomes active again.
    KIdleTime::instance()->catchNextResumeEvent();
}

void IdleMonitor::onResumingFromIdle()
{
    if (m_idle)
        setIdle(false);
}

void IdleMonitor::setIdle(bool idle)
{
    m_idle = idle;
    emit idleChanged(idle);
}

SERVICETEXTINDEX_END_NAMESPACE
