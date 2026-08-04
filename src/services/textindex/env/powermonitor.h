// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef POWERMONITOR_H
#define POWERMONITOR_H

#include "service_textindex_global.h"

#include <QObject>
#include <QTimer>

SERVICETEXTINDEX_BEGIN_NAMESPACE

/**
 * @brief Monitors the system DBus service org.deepin.dde.Power1.
 *
 * Exposes effective OnBattery / power-save state after applying the
 * delay rules from the spec.
 */
class PowerMonitor : public QObject
{
    Q_OBJECT
public:
    explicit PowerMonitor(QObject *parent = nullptr);
    ~PowerMonitor() override;

    void start();
    void stop();

    bool onBattery() const { return m_onBattery; }
    bool powerSaveMode() const { return m_powerSave; }

Q_SIGNALS:
    void effectivePowerChanged(bool onBattery, bool powerSave);

private Q_SLOTS:
    void onBatteryChanged(bool onBattery);
    void onModeChanged(const QString &mode);
    void onPropertiesChanged(const QString &interface, const QVariantMap &changed,
                             const QStringList &invalidated);
    void scheduleEmit();

private:
    bool m_onBattery { false };
    bool m_powerSave { false };
    bool m_started { false };

    bool m_pendingOnBattery { false };
    bool m_pendingPowerSave { false };
    class QTimer *m_batteryEntryTimer { nullptr };
    class QTimer *m_powerSaveEntryTimer { nullptr };
    class QTimer *m_powerSaveExitTimer { nullptr };
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // POWERMONITOR_H
