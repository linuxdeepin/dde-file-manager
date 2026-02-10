// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYNCDBUS_H
#define SYNCDBUS_H

#include <QObject>
#include <QVariantMap>
#include <QThreadPool>
#include <QRunnable>
#include <QAtomicInt>

class SyncTask;

/**
 * @brief DBus service for providing asynchronous filesystem synchronization
 * 
 * This class provides a DBus interface for performing filesystem sync operations
 * without blocking the daemon service. It uses a thread pool to execute sync
 * operations asynchronously and emits signals when operations complete.
 */
class SyncDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.Daemon.Sync")

public:
    explicit SyncDBus(QObject *parent = nullptr);
    ~SyncDBus();

public slots:
    /**
     * @brief Perform filesystem synchronization for a given path
     * 
     * @param path The filesystem path to synchronize
     * @param options Additional options (reserved for future use)
     * @return int Task ID for tracking the operation, or -1 if failed to start
     */
    int SyncFS(const QString &path, const QVariantMap &options = QVariantMap());

    /**
     * @brief Get the current status of running sync operations
     * 
     * @return QVariantMap Status information including active task count
     */
    QVariantMap GetSyncStatus();

signals:
    /**
     * @brief Emitted when a sync operation completes successfully
     * 
     * @param taskId The task ID that was returned by SyncFS
     * @param path The path that was synchronized
     */
    void SyncCompleted(int taskId, const QString &path);

    /**
     * @brief Emitted when a sync operation fails
     * 
     * @param taskId The task ID that was returned by SyncFS
     * @param path The path that failed to synchronize
     * @param errorMessage Description of the error
     */
    void SyncFailed(int taskId, const QString &path, const QString &errorMessage);

private slots:
    /**
     * @brief Handle completion of a sync task
     * 
     * @param task The completed task
     */
    void onSyncTaskCompleted(SyncTask *task);

private:
    /**
     * @brief Generate a unique task ID
     * 
     * @return int Unique task identifier
     */
    int generateTaskId();

    QThreadPool *m_threadPool;
    QAtomicInt m_nextTaskId;
    QHash<int, SyncTask*> m_activeTasks;
};

/**
 * @brief Task class for performing filesystem sync operations
 * 
 * This class implements QRunnable to perform sync operations in background threads.
 * It communicates results back to the main thread via signals.
 */
class SyncTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit SyncTask(int taskId, const QString &path, const QVariantMap &options, QObject *parent = nullptr);

    void run() override;

    int taskId() const { return m_taskId; }
    QString path() const { return m_path; }
    bool isSuccessful() const { return m_success; }
    QString errorMessage() const { return m_errorMessage; }

signals:
    /**
     * @brief Emitted when the sync task completes (success or failure)
     * 
     * @param task Pointer to the completed task
     */
    void taskCompleted(SyncTask *task);

private:
    int m_taskId;
    QString m_path;
    QVariantMap m_options;
    bool m_success;
    QString m_errorMessage;
};

#endif // SYNCDBUS_H 