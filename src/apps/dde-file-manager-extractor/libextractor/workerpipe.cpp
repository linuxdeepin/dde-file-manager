// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workerpipe.h"

#include <QDataStream>
#include <QIODevice>
#include <QFile>
#include <QSocketNotifier>
#include <cstdio>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

EXTRACTOR_BEGIN_NAMESPACE

namespace {
constexpr qsizetype kPipeReadChunkSize = 8192;
}

class WorkerPipePrivate
{
public:
    QSocketNotifier *stdinNotifier = nullptr;
    QByteArray inputBuffer;
    bool waitingForComplete = false;
    qint32 expectedSize = 0;
    bool initialized = false;
    int outputFd = -1;
};

WorkerPipe::WorkerPipe(QObject *parent)
    : QObject(parent), d(new WorkerPipePrivate())
{
}

WorkerPipe::~WorkerPipe()
{
    if (d->stdinNotifier) {
        delete d->stdinNotifier;
    }

    if (d->outputFd >= 0) {
        ::close(d->outputFd);
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

    if (!setupOutputChannel()) {
        fmCritical() << "WorkerPipe::initialize: Failed to set up output channel";
        return false;
    }

    // Create socket notifier to watch stdin
    d->stdinNotifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this);
    connect(d->stdinNotifier, &QSocketNotifier::activated, this, [this]() {
        readFromStdin();
    });

    d->initialized = true;
    fmDebug() << "WorkerPipe: Initialized successfully";

    return true;
}

bool WorkerPipe::readFromStdin()
{
    QByteArray buffer(kPipeReadChunkSize, Qt::Uninitialized);
    const ssize_t bytesRead = ::read(STDIN_FILENO, buffer.data(), static_cast<size_t>(buffer.size()));

    if (bytesRead <= 0) {
        if (hasPendingPartialMessage()) {
            fmWarning() << "WorkerPipe: Stdin closed with incomplete message."
                        << "buffer size:" << d->inputBuffer.size()
                        << "expected payload size:" << d->expectedSize;
        }
        fmInfo() << "WorkerPipe: Stdin closed";
        emit stdinClosed();
        d->stdinNotifier->setEnabled(false);
        return false;
    }

    d->inputBuffer.append(buffer.constData(), static_cast<int>(bytesRead));
    processInputBuffer();
    return true;
}

void WorkerPipe::processInputBuffer()
{
    while (true) {
        if (!d->waitingForComplete) {
            if (d->inputBuffer.size() < static_cast<int>(sizeof(qint32))) {
                return;
            }

            QDataStream sizeStream(d->inputBuffer);
            sizeStream >> d->expectedSize;
            if (d->expectedSize < 0) {
                fmCritical() << "WorkerPipe: Invalid message size:" << d->expectedSize;
                d->stdinNotifier->setEnabled(false);
                emit stdinClosed();
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

        QDataStream messageStream(messageData);
        QVector<QString> filePaths;
        messageStream >> filePaths;

        fmDebug() << "WorkerPipe: Received batch with" << filePaths.size() << "files";
        emit activityDetected();
        emit batchReceived(filePaths);
    }
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

    if (status == ExtractorStatus::Failed) {
        // Fix: failed messages must carry the error payload so the controller
        // does not have to guess or reuse the file path as the error text.
        messageStream << QString::fromUtf8(data);
    }

    QByteArray packetData;
    QDataStream packetStream(&packetData, QIODevice::WriteOnly);
    packetStream << static_cast<qint32>(messageData.size());
    packetData.append(messageData);

    if (!writePacket(packetData)) {
        fmCritical() << "WorkerPipe::sendStatus: Failed to write packet";
        return false;
    }

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
    return sendStatus(ExtractorStatus::Failed, filePath, error.toUtf8());
}

bool WorkerPipe::sendBatchDone()
{
    return sendStatus(ExtractorStatus::BatchDone);
}

bool WorkerPipe::writePacket(const QByteArray &packetData)
{
    qint64 totalWritten = 0;
    while (totalWritten < packetData.size()) {
        const ssize_t bytesWritten = ::write(d->outputFd,
                                             packetData.constData() + totalWritten,
                                             static_cast<size_t>(packetData.size() - totalWritten));
        if (bytesWritten <= 0) {
            return false;
        }

        totalWritten += bytesWritten;
    }

    return true;
}

bool WorkerPipe::hasPendingPartialMessage() const
{
    return d->waitingForComplete || !d->inputBuffer.isEmpty();
}

bool WorkerPipe::setupOutputChannel()
{
    d->outputFd = ::dup(STDOUT_FILENO);
    if (d->outputFd < 0) {
        return false;
    }

    const int nullFd = ::open("/dev/null", O_WRONLY | O_CLOEXEC);
    if (nullFd < 0) {
        ::close(d->outputFd);
        d->outputFd = -1;
        return false;
    }

    // Keep a dedicated fd for binary IPC and sink accidental stdout writes so
    // they cannot be misread as protocol frames or flood the controller's
    // stderr log stream after redirection.
    if (::dup2(nullFd, STDOUT_FILENO) < 0) {
        ::close(nullFd);
        ::close(d->outputFd);
        d->outputFd = -1;
        return false;
    }

    ::close(nullFd);

    if (::fflush(stdout) != 0 && errno != EBADF) {
        fmWarning() << "WorkerPipe::setupOutputChannel: Failed to flush stdout after redirect";
    }

    return true;
}

EXTRACTOR_END_NAMESPACE
