// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workerpipe.h"

#include <QDataStream>
#include <QIODevice>
#include <QFile>
#include <QSocketNotifier>
#include <cstdio>
#include <unistd.h>

EXTRACTOR_BEGIN_NAMESPACE

class WorkerPipePrivate
{
public:
    QSocketNotifier *stdinNotifier = nullptr;
    QByteArray inputBuffer;
    bool waitingForComplete = false;
    qint32 expectedSize = 0;
    bool initialized = false;
};

WorkerPipe::WorkerPipe(QObject *parent)
    : QObject(parent)
    , d(new WorkerPipePrivate())
{
}

WorkerPipe::~WorkerPipe()
{
    if (d->stdinNotifier) {
        delete d->stdinNotifier;
    }
}

bool WorkerPipe::initialize()
{
    if (d->initialized) {
        return true;
    }

    // Set stdin to binary mode
    FILE *stdinFile = freopen(nullptr, "rb", stdin);
    if (!stdinFile) {
        fmCritical() << "WorkerPipe::initialize: Failed to reopen stdin in binary mode";
        return false;
    }

    // Set stdout to binary mode
    FILE *stdoutFile = freopen(nullptr, "wb", stdout);
    if (!stdoutFile) {
        fmCritical() << "WorkerPipe::initialize: Failed to reopen stdout in binary mode";
        return false;
    }

    // Create socket notifier to watch stdin
    d->stdinNotifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this);
    connect(d->stdinNotifier, &QSocketNotifier::activated, this, [this]() {
        // Read available data from stdin
        char buffer[4096];
        ssize_t bytesRead = ::read(STDIN_FILENO, buffer, sizeof(buffer));

        if (bytesRead <= 0) {
            // EOF or error - stdin closed
            fmInfo() << "WorkerPipe: Stdin closed";
            emit stdinClosed();
            d->stdinNotifier->setEnabled(false);
            return;
        }

        // Append to input buffer
        d->inputBuffer.append(buffer, bytesRead);

        // Process complete messages
        while (true) {
            if (!d->waitingForComplete) {
                // Need to read size header first
                if (d->inputBuffer.size() < static_cast<int>(sizeof(qint32))) {
                    break;   // Not enough data for size header
                }

                QDataStream sizeStream(d->inputBuffer);
                sizeStream >> d->expectedSize;
                d->waitingForComplete = true;
            }

            // Check if we have the complete message
            qint32 totalSize = sizeof(qint32) + d->expectedSize;
            if (d->inputBuffer.size() < totalSize) {
                break;   // Not enough data yet
            }

            // Extract the message data (skip size header)
            QByteArray messageData = d->inputBuffer.mid(sizeof(qint32), d->expectedSize);
            d->inputBuffer.remove(0, totalSize);
            d->waitingForComplete = false;
            d->expectedSize = 0;

            // Parse the message
            QDataStream messageStream(messageData);
            QVector<QString> filePaths;
            messageStream >> filePaths;

            fmDebug() << "WorkerPipe: Received batch with" << filePaths.size() << "files";
            emit batchReceived(filePaths);
        }
    });

    d->initialized = true;
    fmDebug() << "WorkerPipe: Initialized successfully";

    return true;
}

bool WorkerPipe::sendStatus(ExtractorStatus status, const QString &filePath, const QByteArray &data)
{
    if (!d->initialized) {
        fmWarning() << "WorkerPipe::sendStatus: Not initialized";
        return false;
    }

    // Serialize the message
    QByteArray messageData;
    QDataStream messageStream(&messageData, QIODevice::WriteOnly);
    messageStream << static_cast<quint8>(status);

    if (status != ExtractorStatus::BatchDone) {
        messageStream << filePath;
    }

    if (status == ExtractorStatus::Data) {
        messageStream << data;
    }

    // Serialize the size header with QDataStream so framing matches the controller side.
    qint32 messageSize = static_cast<qint32>(messageData.size());
    QByteArray headerData;
    QDataStream headerStream(&headerData, QIODevice::WriteOnly);
    headerStream << messageSize;

    if (::write(STDOUT_FILENO, headerData.constData(), headerData.size()) != headerData.size()) {
        fmCritical() << "WorkerPipe::sendStatus: Failed to write size header";
        return false;
    }

    if (::write(STDOUT_FILENO, messageData.constData(), messageSize) != messageSize) {
        fmCritical() << "WorkerPipe::sendStatus: Failed to write message data";
        return false;
    }

    // Flush to ensure the controller receives the data
    ::fflush(stdout);

    fmDebug() << "WorkerPipe::sendStatus: Sent status" << statusToString(status)
              << "for" << filePath << "data size:" << data.size();

    return true;
}

bool WorkerPipe::sendStarted(const QString &filePath)
{
    return sendStatus(ExtractorStatus::Started, filePath);
}

bool WorkerPipe::sendData(const QString &filePath, const QByteArray &data)
{
    return sendStatus(ExtractorStatus::Data, filePath, data);
}

bool WorkerPipe::sendFailed(const QString &filePath, const QString &error)
{
    Q_UNUSED(error)
    return sendStatus(ExtractorStatus::Failed, filePath);
}

bool WorkerPipe::sendBatchDone()
{
    return sendStatus(ExtractorStatus::BatchDone);
}

EXTRACTOR_END_NAMESPACE
