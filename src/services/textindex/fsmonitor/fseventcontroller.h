// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FSEVENTCONTROLLER_H
#define FSEVENTCONTROLLER_H

#include "fseventcollector.h"

#include <QObject>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class FSEventController : public QObject
{
    Q_OBJECT
public:
    explicit FSEventController(QObject *parent = nullptr);

    void setupFSEventCollector();

    bool isEnabled() const;
    void setEnabled(bool enabled);
    void setEnabledNow(bool enabled);

    void startFSMonitoring();
    void stopFSMonitoring();

    void setSilentlyRefreshStarted(bool flag);
    bool silentlyRefreshStarted() const;

private Q_SLOTS:
    void onFilesCreated(const QStringList &paths);
    void onFilesDeleted(const QStringList &paths);
    void onFilesModified(const QStringList &paths);
    void onFilesMoved(const QHash<QString, QString> &movedPaths);
    void onFlushFinished();
    void onConfigChanged();

private:
    void clearCollections();

Q_SIGNALS:
    void requestProcessFileChanges(const QStringList &createdFiles,
                                   const QStringList &modifiedFiles,
                                   const QStringList &deletedFiles);

    void requestProcessFileMoves(const QHash<QString, QString> &movedFiles);

    void monitoring(bool start);
    void requestSlientStart();

private:
    bool m_enabled { false };
    bool m_silentlyFlag { false };
    bool m_lastSilentlyFlag { m_silentlyFlag };
    int m_collectorIntervalSecs { 3 };   // FSEventCollector event collection interval (seconds)
    int m_monitoringStartDelaySecs { 30 };   // FSEventController monitoring start delay (seconds)
    int m_silentStartDelaySecs { 180 };   // FSEventController silent start delay (seconds)
    std::unique_ptr<FSEventCollector> m_fsEventCollector;
    QTimer *m_monitoringStartTimer { nullptr };
    QTimer *m_silentStartTimer { nullptr };
    QTimer *m_stopTimer { nullptr };

    // Collected file events
    QStringList m_collectedCreatedFiles;
    QStringList m_collectedDeletedFiles;
    QStringList m_collectedModifiedFiles;
    QHash<QString, QString> m_collectedMovedFiles;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // FSEVENTCONTROLLER_H
