// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timezonewatcher.h"
#include "universalutils.h"

#include <QFile>
#include <QFileInfo>
#include <QDBusConnection>
#include <QDebug>

// glibc timezone functions
extern "C" {
#include <ctime>
}

namespace dfmbase {

TimezoneWatcher::TimezoneWatcher(QObject *parent)
    : QObject(parent),
      m_initialized(false)
{
}

TimezoneWatcher::~TimezoneWatcher()
{
}

TimezoneWatcher &TimezoneWatcher::instance()
{
    static TimezoneWatcher w;
    return w;
}

void TimezoneWatcher::init()
{
    if (m_initialized)
        return;

    // 1. Initialize timezone
    readAndSetTimezone();

    // 2. Connect D-Bus signals
    connectDbusSignals();

    m_initialized = true;

    qDebug() << "TimezoneWatcher initialized, timezone:" << m_currentTimezone;
}

QString TimezoneWatcher::currentTimezone() const
{
    return m_currentTimezone;
}

bool TimezoneWatcher::isInitialized() const
{
    return m_initialized;
}

void TimezoneWatcher::readAndSetTimezone()
{
    QString tz;

    // 1. Try /etc/timezone first
    tz = readTimezoneFromEtcTimezone();

    // 2. Fallback: /etc/localtime symlink
    if (tz.isEmpty()) {
        tz = readTimezoneFromLocaltime();
    }

    // 3. Final fallback to UTC
    if (tz.isEmpty())
        tz = QStringLiteral("UTC");

    // 4. Update if changed
    if (tz != m_currentTimezone) {
        m_currentTimezone = tz;
        qputenv("TZ", tz.toUtf8());
        tzset();  // Apply immediately
        emit timezoneChanged(tz);
        qInfo() << "Timezone updated to:" << tz;
    }
}

QString TimezoneWatcher::readTimezoneFromEtcTimezone()
{
    QFile tzFile(QStringLiteral("/etc/timezone"));
    if (tzFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString tz = QString::fromUtf8(tzFile.readAll()).trimmed();
        if (!tz.isEmpty())
            return tz;
    }
    return QString();
}

QString TimezoneWatcher::readTimezoneFromLocaltime()
{
    QFileInfo info(QStringLiteral("/etc/localtime"));
    if (info.isSymLink()) {
        QString path = info.symLinkTarget();
        int pos = path.lastIndexOf(QStringLiteral("/zoneinfo/"));
        if (pos >= 0)
            return path.mid(pos + 10);
    }
    return QString();
}

void TimezoneWatcher::connectDbusSignals()
{
    // Check if timedate1 service is available before connecting
    if (!UniversalUtils::checkDbusService(QStringLiteral("org.freedesktop.timedate1")))
        return;

    // Connect directly to org.freedesktop.timedate1 PropertiesChanged signal
    // Note: timedate1 may not be running at init time, signal connection will be lazy
    // D-Bus will deliver matched signals automatically once the service appears
    QDBusConnection::systemBus().connect(
                QStringLiteral("org.freedesktop.timedate1"),
                QStringLiteral("/org/freedesktop/timedate1"),
                QStringLiteral("org.freedesktop.DBus.Properties"),
                QStringLiteral("PropertiesChanged"),
                this,
                SLOT(onPropertiesChanged(QString,QVariantMap,QStringList)));
}

void TimezoneWatcher::onPropertiesChanged(const QString &interface,
                                      const QVariantMap &changed,
                                      const QStringList &invalidated)
{
    Q_UNUSED(invalidated);

    // Only handle org.freedesktop.timedate1 interface
    if (interface == QStringLiteral("org.freedesktop.timedate1")) {
        // Check for Timezone property change
        if (changed.contains(QStringLiteral("Timezone")) || changed.isEmpty()) {
            qInfo() << "Timezone D-Bus properties changed, re-reading timezone";
            readAndSetTimezone();
        }
    }
}

}   // namespace dfmbase
