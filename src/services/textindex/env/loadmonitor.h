// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOADMONITOR_H
#define LOADMONITOR_H

#include "service_textindex_global.h"

#include <QObject>
#include <QString>
#include <QTimer>

SERVICETEXTINDEX_BEGIN_NAMESPACE

/**
 * @brief Samples /proc/stat and /proc/diskstats to maintain a 1-minute
 * rolling average of CPU usage and data-disk busyness.
 *
 * The monitored disk is the block device that backs @c dataPath (the index
 * storage directory), resolved via QStorageInfo at start time.  Only that
 * device's io_ticks are aggregated — not the system disk or every disk.
 */
class LoadMonitor : public QObject
{
    Q_OBJECT
public:
    explicit LoadMonitor(QObject *parent = nullptr);
    ~LoadMonitor() override;

    void start();
    void stop();

    void setDataPath(const QString &path);

    double cpuAvgPercent() const { return m_cpuAvgPercent; }
    double diskBusyPercent() const { return m_diskBusyPercent; }
    double cpuInstantPercent() const { return m_cpuInstantPercent; }
    double diskInstantPercent() const { return m_diskInstantPercent; }

    bool isCpuBelowThreshold() const;
    bool isDiskBelowThreshold() const;

    int cpuThresholdPercent() const { return m_cpuThresholdPercent; }
    int diskThresholdPercent() const { return m_diskThresholdPercent; }

Q_SIGNALS:
    void loadChanged(double cpuAvgPercent, double diskBusyPercent);

private:
    void sample();
    bool readProcStat(qint64 &userJiffies, qint64 &guestJiffies, qint64 &totalJiffies) const;
    bool readProcDiskstats(qint64 &ioTicks) const;
    void resolveDataDisk();

    class QTimer *m_timer { nullptr };
    int m_sampleIntervalSecs { 5 };

    QString m_dataPath;
    QString m_diskDeviceName;

    struct Sample {
        qint64 cpuUser { 0 };
        qint64 cpuGuest { 0 };
        qint64 cpuTotal { 0 };
        qint64 diskIo { 0 };
    };

    QList<Sample> m_window;

    double m_cpuAvgPercent { 0.0 };
    double m_diskBusyPercent { 0.0 };
    double m_cpuInstantPercent { 0.0 };
    double m_diskInstantPercent { 0.0 };

    int m_cpuThresholdPercent { 30 };
    int m_diskThresholdPercent { 50 };
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // LOADMONITOR_H
