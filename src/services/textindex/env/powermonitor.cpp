// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "powermonitor.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace {
constexpr auto kPowerService = "org.deepin.dde.Power1";
constexpr auto kPowerPath = "/org/deepin/dde/Power1";
constexpr auto kPowerIface = "org.deepin.dde.Power1";
}

PowerMonitor::PowerMonitor(QObject *parent)
    : QObject(parent)
{
    m_batteryEntryTimer = new QTimer(this);
    m_batteryEntryTimer->setSingleShot(true);
    m_batteryEntryTimer->setInterval(500);
    connect(m_batteryEntryTimer, &QTimer::timeout, this, [this]() {
        m_onBattery = m_pendingOnBattery;
        scheduleEmit();
    });

    m_powerSaveEntryTimer = new QTimer(this);
    m_powerSaveEntryTimer->setSingleShot(true);
    m_powerSaveEntryTimer->setInterval(500);
    connect(m_powerSaveEntryTimer, &QTimer::timeout, this, [this]() {
        m_powerSave = m_pendingPowerSave;
        scheduleEmit();
    });

    m_powerSaveExitTimer = new QTimer(this);
    m_powerSaveExitTimer->setSingleShot(true);
    m_powerSaveExitTimer->setInterval(500);
    connect(m_powerSaveExitTimer, &QTimer::timeout, this, [this]() {
        m_powerSave = m_pendingPowerSave;
        scheduleEmit();
    });
}

PowerMonitor::~PowerMonitor() = default;

void PowerMonitor::start()
{
    if (m_started)
        return;
    m_started = true;

    auto bus = QDBusConnection::systemBus();
    QDBusInterface iface(kPowerService, kPowerPath, kPowerIface, bus);

    const QVariant batteryProp = iface.property("OnBattery");
    if (batteryProp.isValid()) {
        m_onBattery = batteryProp.toBool();
        m_pendingOnBattery = m_onBattery;
    }

    const QVariant modeProp = iface.property("Mode");
    if (modeProp.isValid()) {
        m_powerSave = (modeProp.toString() == QLatin1String("powersave"));
        m_pendingPowerSave = m_powerSave;
    }

    bus.connect(kPowerService, kPowerPath, "org.freedesktop.DBus.Properties",
                "PropertiesChanged", this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
}

void PowerMonitor::onPropertiesChanged(const QString &interface,
                                       const QVariantMap &changed,
                                       const QStringList &invalidated)
{
    Q_UNUSED(interface)
    Q_UNUSED(invalidated)

    if (changed.contains(QStringLiteral("OnBattery")))
        onBatteryChanged(changed.value(QStringLiteral("OnBattery")).toBool());

    if (changed.contains(QStringLiteral("Mode")))
        onModeChanged(changed.value(QStringLiteral("Mode")).toString());
}

void PowerMonitor::stop()
{
    if (!m_started)
        return;
    m_started = false;

    auto bus = QDBusConnection::systemBus();
    bus.disconnect(kPowerService, kPowerPath, "org.freedesktop.DBus.Properties",
                   "PropertiesChanged", this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));

    m_batteryEntryTimer->stop();
    m_powerSaveEntryTimer->stop();
    m_powerSaveExitTimer->stop();
}

void PowerMonitor::onBatteryChanged(bool onBattery)
{
    if (onBattery == m_pendingOnBattery)
        return;
    m_pendingOnBattery = onBattery;
    if (onBattery)
        m_batteryEntryTimer->start();
    else {
        m_batteryEntryTimer->stop();
        m_onBattery = false;
        scheduleEmit();
    }
}

void PowerMonitor::onModeChanged(const QString &mode)
{
    const bool ps = (mode == QLatin1String("powersave"));
    if (ps == m_pendingPowerSave)
        return;
    m_pendingPowerSave = ps;
    if (ps) {
        m_powerSaveExitTimer->stop();
        m_powerSaveEntryTimer->start();
    } else {
        m_powerSaveEntryTimer->stop();
        m_powerSaveExitTimer->start();
    }
}

void PowerMonitor::scheduleEmit()
{
    emit effectivePowerChanged(m_onBattery, m_powerSave);
}

SERVICETEXTINDEX_END_NAMESPACE
