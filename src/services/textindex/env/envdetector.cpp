// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "envdetector.h"
#include "powermonitor.h"
#include "idlemonitor.h"
#include "loadmonitor.h"

SERVICETEXTINDEX_BEGIN_NAMESPACE

EnvDetector &EnvDetector::instance()
{
    static EnvDetector self;
    return self;
}

EnvDetector::EnvDetector(QObject *parent)
    : QObject(parent)
{
    m_powerMonitor = new PowerMonitor(this);
    m_idleMonitor = new IdleMonitor(this);
    m_loadMonitor = new LoadMonitor(this);

    connect(m_powerMonitor, &PowerMonitor::effectivePowerChanged, this, [this]() {
        recomputeState();
    });
    connect(m_idleMonitor, &IdleMonitor::idleChanged, this, [this]() {
        recomputeState();
    });
    connect(m_loadMonitor, &LoadMonitor::loadChanged, this, [this]() {
        recomputeState();
    });
}

EnvDetector::~EnvDetector() = default;

void EnvDetector::start()
{
    if (m_running)
        return;
    m_running = true;
    m_powerMonitor->start();
    m_idleMonitor->start();
    m_loadMonitor->start();
    recomputeState();
}

void EnvDetector::stop()
{
    if (!m_running)
        return;
    m_running = false;
    m_powerMonitor->stop();
    m_idleMonitor->stop();
    m_loadMonitor->stop();
}

void EnvDetector::setDataPath(const QString &path)
{
    m_loadMonitor->setDataPath(path);
}

EnvState EnvDetector::currentState() const
{
    return m_state;
}

void EnvDetector::recomputeState()
{
    EnvState prev = m_state;

    m_state.onBattery = m_powerMonitor->onBattery();
    m_state.powerSaveMode = m_powerMonitor->powerSaveMode();
    m_state.idle = m_idleMonitor->idle()
            && m_loadMonitor->isCpuBelowThreshold()
            && m_loadMonitor->isDiskBelowThreshold();

    if (prev.onBattery != m_state.onBattery
            || prev.powerSaveMode != m_state.powerSaveMode
            || prev.idle != m_state.idle) {
        emit envStateChanged(m_state);
    }
}

SERVICETEXTINDEX_END_NAMESPACE
