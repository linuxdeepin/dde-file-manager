// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENVDETECTOR_H
#define ENVDETECTOR_H

#include "service_textindex_global.h"

#include <QObject>
#include <QString>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class PowerMonitor;
class IdleMonitor;
class LoadMonitor;

/**
 * @brief Aggregated environment state consumed by the task scheduler.
 *
 * Each field reflects the *effective* state after applying the required
 * time-delay rules. The scheduler never reads raw sensor data directly.
 */
struct EnvState
{
    bool onBattery { false };
    bool powerSaveMode { false };
    bool idle { false };

    bool isPowerOk() const { return !onBattery; }
    bool isPowerSaveOff() const { return !powerSaveMode; }
    bool isIdleOk() const { return idle; }
};

/**
 * @brief Unified environment-detection entry point.
 *
 * Owns PowerMonitor, IdleMonitor and LoadMonitor, applies the delay rules
 * and exposes a single EnvState + change signal to the scheduler.
 */
class EnvDetector : public QObject
{
    Q_OBJECT
public:
    static EnvDetector &instance();

    ~EnvDetector() override;

    void start();
    void stop();

    /// Set the directory whose underlying disk the LoadMonitor should track.
    void setDataPath(const QString &path);

    EnvState currentState() const;

Q_SIGNALS:
    void envStateChanged(const EnvState &state);

private:
    explicit EnvDetector(QObject *parent = nullptr);

    void recomputeState();

    PowerMonitor *m_powerMonitor { nullptr };
    IdleMonitor *m_idleMonitor { nullptr };
    LoadMonitor *m_loadMonitor { nullptr };

    EnvState m_state;
    bool m_running { false };
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // ENVDETECTOR_H
