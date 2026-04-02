// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTROLLERPIPE_H
#define CONTROLLERPIPE_H

#include "extractor_global.h"
#include "extractortypes.h"

#include <QObject>
#include <QProcess>
#include <QSharedPointer>

EXTRACTOR_BEGIN_NAMESPACE

class ControllerPipePrivate;

/**
 * @brief ControllerPipe manages communication with the extractor subprocess.
 *
 * This class is used by the main process (e.g., textindex service) to:
 * 1. Start the extractor subprocess
 * 2. Send batch extraction requests
 * 3. Receive extraction results via signals
 * 4. Handle subprocess lifecycle (crashes, timeouts)
 */
class ControllerPipe : public QObject
{
    Q_OBJECT

public:
    explicit ControllerPipe(QObject *parent = nullptr);
    ~ControllerPipe() override;

    /**
     * @brief Start the extractor subprocess
     * @param extractorPath Path to the extractor executable
     * @param pluginPath Optional path to plugin directory
     * @return true if started successfully
     */
    bool start(const QString &extractorPath, const QString &pluginPath = QString());

    /**
     * @brief Stop the extractor subprocess
     */
    void stop();

    /**
     * @brief Check if the extractor is running
     */
    bool isRunning() const;

    /**
     * @brief Send a batch of file paths for extraction
     * @param filePaths List of file paths to extract
     * @return true if the request was sent successfully
     */
    bool extractBatch(const QVector<QString> &filePaths);

    /**
     * @brief Get the process ID of the extractor
     * @return Process ID or -1 if not running
     */
    qint64 processId() const;

signals:
    /**
     * @brief Emitted when extraction starts for a file
     * @param filePath The file being processed
     */
    void extractionStarted(const QString &filePath);

    /**
     * @brief Emitted when extraction completes successfully
     * @param filePath The file that was processed
     * @param data The extracted content
     */
    void extractionFinished(const QString &filePath, const QByteArray &data);

    /**
     * @brief Emitted when extraction fails
     * @param filePath The file that failed
     * @param error Error description
     */
    void extractionFailed(const QString &filePath, const QString &error);

    /**
     * @brief Emitted when a batch extraction is complete
     */
    void batchFinished();

    /**
     * @brief Emitted when the extractor process crashes
     */
    void processCrashed();

    /**
     * @brief Emitted when the extractor process exits normally
     */
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /**
     * @brief Emitted when an error occurs
     * @param error Error description
     */
    void errorOccurred(const QString &error);

private:
    QScopedPointer<ControllerPipePrivate> d;
};

EXTRACTOR_END_NAMESPACE

#endif   // CONTROLLERPIPE_H
