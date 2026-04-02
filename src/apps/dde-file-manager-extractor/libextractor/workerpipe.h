// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKERPIPE_H
#define WORKERPIPE_H

#include "extractor_global.h"
#include "extractortypes.h"

#include <QObject>

EXTRACTOR_BEGIN_NAMESPACE

class WorkerPipePrivate;

/**
 * @brief WorkerPipe handles communication from the extractor subprocess side.
 *
 * This class is used by the extractor subprocess to:
 * 1. Receive extraction requests from stdin
 * 2. Send status updates and results to stdout
 * 3. Handle message framing with QDataStream transactions
 */
class WorkerPipe : public QObject
{
    Q_OBJECT

public:
    explicit WorkerPipe(QObject *parent = nullptr);
    ~WorkerPipe() override;

    /**
     * @brief Initialize the worker pipe
     * @return true if initialization succeeded
     */
    bool initialize();

    /**
     * @brief Send a status message to the controller
     * @param status The status code
     * @param filePath Optional file path (for Started/Finished/Failed/Data)
     * @param data Optional data (for Data status)
     * @return true if sent successfully
     */
    bool sendStatus(ExtractorStatus status, const QString &filePath = QString(),
                    const QByteArray &data = QByteArray());

    /**
     * @brief Send extraction started notification
     */
    bool sendStarted(const QString &filePath);

    /**
     * @brief Send extraction finished with data
     */
    bool sendData(const QString &filePath, const QByteArray &data);

    /**
     * @brief Send extraction failed notification
     */
    bool sendFailed(const QString &filePath, const QString &error = QString());

    /**
     * @brief Send batch done notification
     */
    bool sendBatchDone();

signals:
    /**
     * @brief Emitted when a complete batch request is received from stdin.
     *
     * This is used by the extractor app to track idle time based on actual
     * tasks rather than low-level notifier wakeups or partial pipe reads.
     */
    void activityDetected();

    /**
     * @brief Emitted when a batch request is received
     * @param filePaths List of files to extract
     */
    void batchReceived(const QVector<QString> &filePaths);

    /**
     * @brief Emitted when stdin is closed (parent process terminated)
     */
    void stdinClosed();

private:
    bool readFromStdin();
    void processInputBuffer();
    bool writePacket(const QByteArray &packetData);
    bool hasPendingPartialMessage() const;
    bool setupOutputChannel();

    QScopedPointer<WorkerPipePrivate> d;
};

EXTRACTOR_END_NAMESPACE

#endif   // WORKERPIPE_H
