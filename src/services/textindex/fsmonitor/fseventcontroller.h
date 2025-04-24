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

private Q_SLOTS:
    void onFilesCreated(const QStringList &paths);
    void onFilesDeleted(const QStringList &paths);
    void onFilesModified(const QStringList &paths);
    void onFlushFinished();

private:
    void clearCollections();
    void startFSMonitoring();
    void stopFSMonitoring();

Q_SIGNALS:
    void requestProcessFileChanges(const QStringList &createdFiles,
                                   const QStringList &modifiedFiles,
                                   const QStringList &deletedFiles);

private:
    bool m_enabled { false };
    std::unique_ptr<FSEventCollector> m_fsEventCollector;
    QTimer *m_stopTimer;

    // Collected file events
    QStringList m_collectedCreatedFiles;
    QStringList m_collectedDeletedFiles;
    QStringList m_collectedModifiedFiles;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // FSEVENTCONTROLLER_H
