// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "controllerpipe.h"

#include <QDataStream>
#include <QProcess>
#include <QTimer>

EXTRACTOR_BEGIN_NAMESPACE

class ControllerPipePrivate
{
public:
    QProcess *process = nullptr;
    QByteArray inputBuffer;
    bool waitingForComplete = false;
    qint32 expectedSize = 0;

    void clearState()
    {
        inputBuffer.clear();
        waitingForComplete = false;
        expectedSize = 0;
    }
};

ControllerPipe::ControllerPipe(QObject *parent)
    : QObject(parent), d(new ControllerPipePrivate())
{
}

ControllerPipe::~ControllerPipe()
{
    stop();
}

bool ControllerPipe::start(const QString &extractorPath, const QString &pluginPath)
{
    if (isRunning()) {
        fmWarning() << "ControllerPipe::start: Process already running";
        return false;
    }

    if (extractorPath.isEmpty()) {
        fmCritical() << "ControllerPipe::start: Extractor path is empty";
        emit errorOccurred("Extractor path is empty");
        return false;
    }

    d->process = new QProcess(this);

    // Connect process signals
    connect(d->process, &QProcess::readyReadStandardOutput,
            this, &ControllerPipe::handleProcessOutput);

    connect(d->process, &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError error) {
                if (hasPendingPartialMessage()) {
                    fmWarning() << "ControllerPipe: Process error occurred with incomplete message."
                                << "buffer size:" << d->inputBuffer.size()
                                << "expected payload size:" << d->expectedSize;
                }
                fmCritical() << "ControllerPipe: Process error:" << error;
                emit errorOccurred(QString("Process error: %1").arg(static_cast<int>(error)));
            });

    connect(d->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                QProcess *finishedProcess = qobject_cast<QProcess *>(sender());
                if (hasPendingPartialMessage()) {
                    fmWarning() << "ControllerPipe: Process finished with incomplete message."
                                << "buffer size:" << d->inputBuffer.size()
                                << "expected payload size:" << d->expectedSize;
                }
                fmInfo() << "ControllerPipe: Process finished with exit code:" << exitCode
                         << "status:" << exitStatus;

                if (exitStatus == QProcess::CrashExit) {
                    emit processCrashed();
                }

                emit processFinished(exitCode, exitStatus);

                if (d->process == finishedProcess) {
                    d->process = nullptr;
                }

                if (finishedProcess) {
                    finishedProcess->deleteLater();
                }
            });

    // Prepare arguments
    QStringList arguments;
    if (!pluginPath.isEmpty()) {
        arguments << "--plugin-path" << pluginPath;
    }

    // Start the process
    fmInfo() << "ControllerPipe: Starting extractor:" << extractorPath
             << "arguments:" << arguments;

    d->process->start(extractorPath, arguments, QIODevice::ReadWrite);

    if (!d->process->waitForStarted(5000)) {
        fmCritical() << "ControllerPipe: Failed to start process:"
                     << d->process->errorString();
        emit errorOccurred(QString("Failed to start process: %1").arg(d->process->errorString()));
        d->process->deleteLater();
        d->process = nullptr;
        return false;
    }

    fmInfo() << "ControllerPipe: Extractor started successfully, PID:" << d->process->processId();
    return true;
}

void ControllerPipe::handleProcessOutput()
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    if (!process) {
        process = d->process;
    }
    if (!process) {
        return;
    }

    d->inputBuffer.append(process->readAllStandardOutput());
    processInputBuffer();
}

void ControllerPipe::processInputBuffer()
{
    while (true) {
        if (!d->waitingForComplete) {
            if (d->inputBuffer.size() < static_cast<int>(sizeof(qint32))) {
                return;
            }

            QDataStream sizeStream(d->inputBuffer);
            sizeStream >> d->expectedSize;
            if (d->expectedSize < 0) {
                fmCritical() << "ControllerPipe: Invalid message size:" << d->expectedSize;
                d->clearState();
                emit errorOccurred(QStringLiteral("Invalid extractor message size"));
                return;
            }

            d->waitingForComplete = true;
        }

        const qint32 totalSize = static_cast<qint32>(sizeof(qint32)) + d->expectedSize;
        const auto bufferSize = d->inputBuffer.size();
        if (bufferSize < totalSize) {
            return;
        }

        const QByteArray messageData = d->inputBuffer.mid(sizeof(qint32), d->expectedSize);
        d->inputBuffer.remove(0, totalSize);
        d->waitingForComplete = false;
        d->expectedSize = 0;

        handleStatusMessage(messageData);
    }
}

void ControllerPipe::handleStatusMessage(const QByteArray &messageData)
{
    QDataStream messageStream(messageData);
    quint8 statusByte = 0;
    messageStream >> statusByte;
    const ExtractorStatus status = static_cast<ExtractorStatus>(statusByte);

    QString filePath;
    QByteArray data;
    QString error;

    if (status != ExtractorStatus::BatchDone) {
        messageStream >> filePath;
    }

    if (status == ExtractorStatus::Data) {
        messageStream >> data;
    } else if (status == ExtractorStatus::Failed) {
        // Fix: failed packets now carry an explicit error string after filePath.
        messageStream >> error;
    }

    switch (status) {
    case ExtractorStatus::Started:
        fmDebug() << "ControllerPipe: Extraction started for:" << filePath;
        emit extractionStarted(filePath);
        break;

    case ExtractorStatus::Finished:
        fmDebug() << "ControllerPipe: Extraction finished for:" << filePath;
        emit extractionFinished(filePath, data);
        break;

    case ExtractorStatus::Failed:
        fmWarning() << "ControllerPipe: Extraction failed for:" << filePath << "error:" << error;
        emit extractionFailed(filePath, error);
        break;

    case ExtractorStatus::Data:
        fmDebug() << "ControllerPipe: Received data for:" << filePath
                  << "size:" << data.size();
        emit extractionFinished(filePath, data);
        break;

    case ExtractorStatus::BatchDone:
        fmDebug() << "ControllerPipe: Batch completed";
        emit batchFinished();
        break;
    }
}

void ControllerPipe::stop()
{
    if (d->process) {
        QProcess *process = d->process;
        d->process = nullptr;

        fmInfo() << "ControllerPipe: Stopping extractor process";

        // Close write channel to signal EOF to the subprocess
        process->closeWriteChannel();

        // Wait for the process to finish gracefully
        if (!process->waitForFinished(3000)) {
            fmWarning() << "ControllerPipe: Process did not finish gracefully, terminating";
            process->terminate();

            if (!process->waitForFinished(2000)) {
                fmWarning() << "ControllerPipe: Process did not terminate, killing";
                process->kill();
                process->waitForFinished(1000);
            }
        }

        process->deleteLater();
    }

    d->clearState();
}

bool ControllerPipe::hasPendingPartialMessage() const
{
    return d->waitingForComplete || !d->inputBuffer.isEmpty();
}

bool ControllerPipe::isRunning() const
{
    return d->process && d->process->state() == QProcess::Running;
}

bool ControllerPipe::extractBatch(const QVector<QString> &filePaths)
{
    if (!isRunning()) {
        fmWarning() << "ControllerPipe::extractBatch: Process not running";
        return false;
    }

    if (filePaths.isEmpty()) {
        fmWarning() << "ControllerPipe::extractBatch: Empty file paths";
        return false;
    }

    fmDebug() << "ControllerPipe::extractBatch: Sending" << filePaths.size() << "files";

    // Serialize the request
    QByteArray messageData;
    QDataStream messageStream(&messageData, QIODevice::WriteOnly);
    messageStream << filePaths;

    // Write size header + message
    QByteArray packetData;
    QDataStream packetStream(&packetData, QIODevice::WriteOnly);
    packetStream << static_cast<qint32>(messageData.size());
    packetData.append(messageData);

    qint64 bytesWritten = d->process->write(packetData);
    if (bytesWritten != packetData.size()) {
        fmCritical() << "ControllerPipe::extractBatch: Failed to write complete message";
        return false;
    }

    d->process->waitForBytesWritten();
    return true;
}

qint64 ControllerPipe::processId() const
{
    return d->process ? d->process->processId() : -1;
}

EXTRACTOR_END_NAMESPACE
