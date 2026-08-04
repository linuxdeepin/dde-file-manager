// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadmonitor.h"
#include "utils/textindexconfig.h"

#include <QFile>
#include <QRegularExpression>
#include <QStorageInfo>
#include <QTextStream>

SERVICETEXTINDEX_BEGIN_NAMESPACE

LoadMonitor::LoadMonitor(QObject *parent)
    : QObject(parent)
{
    m_sampleIntervalSecs = TextIndexConfig::instance().loadSampleIntervalSeconds();
    m_cpuThresholdPercent = TextIndexConfig::instance().cpuLoadThresholdPercent();
    m_diskThresholdPercent = TextIndexConfig::instance().diskBusyThresholdPercent();

    m_timer = new QTimer(this);
    m_timer->setInterval(m_sampleIntervalSecs * 1000);
    connect(m_timer, &QTimer::timeout, this, &LoadMonitor::sample);
}

LoadMonitor::~LoadMonitor() = default;

void LoadMonitor::start()
{
    resolveDataDisk();
    m_timer->start();
    sample();
}

void LoadMonitor::stop()
{
    m_timer->stop();
}

void LoadMonitor::setDataPath(const QString &path)
{
    m_dataPath = path;
}

bool LoadMonitor::readProcStat(qint64 &userJiffies, qint64 &guestJiffies, qint64 &totalJiffies) const
{
    QFile file(QStringLiteral("/proc/stat"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    const QString line = QTextStream(&file).readLine();
    const auto parts = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (parts.size() < 5)
        return false;

    userJiffies = parts[1].toLongLong();
    guestJiffies = (parts.size() >= 10) ? parts[9].toLongLong() : 0;
    totalJiffies = 0;
    for (int i = 1; i < qMin(parts.size(), 9); ++i)
        totalJiffies += parts[i].toLongLong();
    return true;
}

void LoadMonitor::resolveDataDisk()
{
    if (m_dataPath.isEmpty())
        return;

    const QStorageInfo storage(m_dataPath);
    const QString device = storage.device();   // e.g. "/dev/sda3" or "/dev/nvme0n1p2"
    if (device.startsWith(QStringLiteral("/dev/"))) {
        m_diskDeviceName = device.mid(5);
    }
}

bool LoadMonitor::readProcDiskstats(qint64 &ioTicks) const
{
    if (m_diskDeviceName.isEmpty())
        return false;

    QFile file(QStringLiteral("/proc/diskstats"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    const QString content = QString::fromUtf8(file.readAll());
    const auto lines = content.split(QLatin1Char('\n'));

    QString bestMatch;
    qint64 bestTicks = 0;
    for (const QString &line : lines) {
        const auto parts = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (parts.size() < 13)
            continue;
        // parts[0]=major, [1]=minor, [2]=device name, ..., [12]=io_ticks (ms)
        const QString &devName = parts[2];

        // Exact match: whole-disk mount (e.g. /dev/sda) or LVM (e.g. /dev/dm-0)
        if (m_diskDeviceName == devName) {
            if (devName.size() > bestMatch.size()) {
                bestMatch = devName;
                bestTicks = parts[12].toLongLong();
            }
            continue;
        }

        // Partition match: devName is a prefix of m_diskDeviceName
        if (m_diskDeviceName.startsWith(devName)) {
            const QChar next = m_diskDeviceName.at(devName.size());
            if (next.isDigit() || next == QLatin1Char('p')) {
                if (devName.size() > bestMatch.size()) {
                    bestMatch = devName;
                    bestTicks = parts[12].toLongLong();
                }
            }
        }
    }

    if (bestMatch.isEmpty())
        return false;

    ioTicks = bestTicks;
    return true;
}

void LoadMonitor::sample()
{
    qint64 userJiffies = 0, guestJiffies = 0, totalJiffies = 0;
    if (!readProcStat(userJiffies, guestJiffies, totalJiffies))
        return;

    qint64 ioTicks = 0;
    const bool diskOk = readProcDiskstats(ioTicks);

    Sample s;
    s.cpuUser = userJiffies;
    s.cpuGuest = guestJiffies;
    s.cpuTotal = totalJiffies;
    s.diskIo = ioTicks;

    m_window.append(s);
    const int maxSamples = qMax(1, 60 / m_sampleIntervalSecs);
    while (m_window.size() > maxSamples)
        m_window.removeFirst();

    if (m_window.size() < 2)
        return;

    // Compute the 1-minute rolling average over the entire window.
    const Sample &oldest = m_window.first();
    const Sample &newest = m_window.last();

    const qint64 winTotal = newest.cpuTotal - oldest.cpuTotal;
    if (winTotal > 0) {
        const qint64 winUser = newest.cpuUser - oldest.cpuUser;
        const qint64 winGuest = newest.cpuGuest - oldest.cpuGuest;
        const double winUsr = winUser - winGuest;
        m_cpuAvgPercent = qBound(0.0, (winUsr / static_cast<double>(winTotal)) * 100.0, 100.0);
    }

    const qint64 winIo = newest.diskIo - oldest.diskIo;
    const qint64 elapsedMs = static_cast<qint64>(m_window.size() - 1) * m_sampleIntervalSecs * 1000;
    if (diskOk && elapsedMs > 0 && winIo >= 0)
        m_diskBusyPercent = qBound(0.0, (winIo / static_cast<double>(elapsedMs)) * 100.0, 100.0);

    // Instantaneous values from the last two samples.
    if (m_window.size() >= 2) {
        const Sample &prev = m_window[m_window.size() - 2];
        const qint64 instTotal = newest.cpuTotal - prev.cpuTotal;
        if (instTotal > 0) {
            const qint64 instUser = newest.cpuUser - prev.cpuUser;
            const qint64 instGuest = newest.cpuGuest - prev.cpuGuest;
            m_cpuInstantPercent = qBound(0.0, ((instUser - instGuest) / static_cast<double>(instTotal)) * 100.0, 100.0);
        }
        const qint64 instIo = newest.diskIo - prev.diskIo;
        const qint64 instMs = m_sampleIntervalSecs * 1000;
        if (diskOk && instMs > 0 && instIo >= 0)
            m_diskInstantPercent = qBound(0.0, (instIo / static_cast<double>(instMs)) * 100.0, 100.0);
    }

    emit loadChanged(m_cpuAvgPercent, m_diskBusyPercent);
}

bool LoadMonitor::isCpuBelowThreshold() const
{
    return m_cpuAvgPercent <= m_cpuThresholdPercent;
}

bool LoadMonitor::isDiskBelowThreshold() const
{
    return m_diskBusyPercent <= m_diskThresholdPercent;
}

SERVICETEXTINDEX_END_NAMESPACE
