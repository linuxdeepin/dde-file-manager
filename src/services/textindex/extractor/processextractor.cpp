// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processextractor.h"
#include "config.h"

#include <controllerpipe.h>

#include <QEventLoop>
#include <QThread>
#include <QTimer>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace {

constexpr int kExtractorRequestTimeoutMs = 120000;
constexpr int kExtractorIdleShutdownMs = 60000;

struct ActiveExtraction
{
    QString filePath;
    IndexExtractionResult result;
    QEventLoop *loop { nullptr };
};

class ProcessExtractorProxy : public QObject
{
public:
    explicit ProcessExtractorProxy(QString extractorPath, QObject *parent = nullptr)
        : QObject(parent),
          m_extractorPath(std::move(extractorPath)),
          m_pipe(new EXTRACTOR_NAMESPACE::ControllerPipe(this))
    {
        m_requestTimeoutTimer.setSingleShot(true);
        m_idleShutdownTimer.setSingleShot(true);

        connect(m_pipe, &EXTRACTOR_NAMESPACE::ControllerPipe::extractionFinished, this,
                [this](const QString &path, const QByteArray &data) {
                    if (!m_activeExtraction || path != m_activeExtraction->filePath) {
                        return;
                    }

                    fmDebug() << "ProcessExtractorProxy: extraction finished for:" << path
                              << "size:" << data.size();
                    finishActiveExtraction({ true, QString::fromUtf8(data).trimmed(), QString() });
                });

        connect(m_pipe, &EXTRACTOR_NAMESPACE::ControllerPipe::extractionFailed, this,
                [this](const QString &path, const QString &error) {
                    if (!m_activeExtraction || path != m_activeExtraction->filePath) {
                        return;
                    }

                    fmWarning() << "ProcessExtractorProxy: extraction failed for:" << path
                                << "error:" << error;
                    finishActiveExtraction({ false, QString(), error });
                });

        connect(m_pipe, &EXTRACTOR_NAMESPACE::ControllerPipe::errorOccurred, this,
                [this](const QString &error) {
                    if (m_stoppingPipe) {
                        return;
                    }

                    if (!m_activeExtraction) {
                        fmWarning() << "ProcessExtractorProxy: extractor reported error without active request:"
                                    << error;
                        return;
                    }

                    fmWarning() << "ProcessExtractorProxy: extractor error for:"
                                << m_activeExtraction->filePath << "error:" << error;
                    stopPipe();
                    finishActiveExtraction({ false, QString(), error });
                });

        connect(m_pipe, &EXTRACTOR_NAMESPACE::ControllerPipe::processCrashed, this, [this]() {
            if (m_stoppingPipe) {
                return;
            }

            if (!m_activeExtraction) {
                fmWarning() << "ProcessExtractorProxy: extractor process crashed without active request";
                stopPipe();
                return;
            }

            fmWarning() << "ProcessExtractorProxy: extractor process crashed during request for:"
                        << m_activeExtraction->filePath;
            stopPipe();
            finishActiveExtraction({ false, QString(), QStringLiteral("Extractor process crashed") });
        });

        connect(m_pipe, &EXTRACTOR_NAMESPACE::ControllerPipe::processFinished, this,
                [this](int exitCode, QProcess::ExitStatus exitStatus) {
                    if (m_stoppingPipe) {
                        return;
                    }

                    if (!m_activeExtraction || exitStatus == QProcess::CrashExit) {
                        return;
                    }

                    fmWarning() << "ProcessExtractorProxy: extractor process finished unexpectedly during request for:"
                                << m_activeExtraction->filePath << "exitCode:" << exitCode
                                << "exitStatus:" << exitStatus;
                    stopPipe();
                    finishActiveExtraction({ false, QString(), QStringLiteral("Extractor process finished unexpectedly") });
                });

        connect(&m_requestTimeoutTimer, &QTimer::timeout, this, [this]() {
            if (!m_activeExtraction) {
                return;
            }

            fmWarning() << "ProcessExtractorProxy: request timed out for:" << m_activeExtraction->filePath;
            stopPipe();
            finishActiveExtraction({ false, QString(), QStringLiteral("Extractor request timed out") });
        });

        connect(&m_idleShutdownTimer, &QTimer::timeout, this, [this]() {
            if (!m_activeExtraction) {
                fmInfo() << "ProcessExtractorProxy: stopping idle extractor process after timeout";
                stopPipe();
            }
        });
    }

    IndexExtractionResult extractSync(const QString &filePath, size_t maxBytes)
    {
        Q_ASSERT(QThread::currentThread() == thread());
        Q_UNUSED(maxBytes)

        if (m_activeExtraction) {
            fmWarning() << "ProcessExtractorProxy: received overlapping extract request for:" << filePath;
            return { false, QString(), QStringLiteral("Extractor is busy processing another file") };
        }

        m_idleShutdownTimer.stop();

        if (!ensurePipeStarted()) {
            return { false, QString(), QStringLiteral("Failed to start dde-file-manager-extractor") };
        }

        QEventLoop loop;
        ActiveExtraction extraction;
        extraction.filePath = filePath;
        extraction.loop = &loop;
        m_activeExtraction = &extraction;

        m_requestTimeoutTimer.start(kExtractorRequestTimeoutMs);
        if (!m_pipe->extractBatch({ filePath })) {
            fmWarning() << "ProcessExtractorProxy: failed to send extractor request for:" << filePath;
            stopPipe();
            finishActiveExtraction({ false, QString(), QStringLiteral("Failed to send extractor request") });
        }

        if (m_activeExtraction) {
            loop.exec();
        }

        return extraction.result;
    }

    void shutdown()
    {
        if (QThread::currentThread() != thread()) {
            QMetaObject::invokeMethod(this, [this]() {
                shutdown();
            }, Qt::BlockingQueuedConnection);
            return;
        }

        m_idleShutdownTimer.stop();
        m_requestTimeoutTimer.stop();

        finishActiveExtraction({ false, QString(), QStringLiteral("Process extractor shutting down") });
        stopPipe();
    }

private:
    bool ensurePipeStarted()
    {
        Q_ASSERT(QThread::currentThread() == thread());

        if (m_pipe->isRunning()) {
            return true;
        }

        if (m_extractorPath.isEmpty()) {
            fmCritical() << "ProcessExtractorProxy: extractor path is empty";
            return false;
        }

        if (!m_pipe->start(m_extractorPath)) {
            fmCritical() << "ProcessExtractorProxy: failed to start extractor process";
            return false;
        }

        fmInfo() << "ProcessExtractorProxy: extractor process started, pid:" << m_pipe->processId();
        return true;
    }

    void finishActiveExtraction(const IndexExtractionResult &result)
    {
        Q_ASSERT(QThread::currentThread() == thread());

        if (!m_activeExtraction) {
            return;
        }

        m_requestTimeoutTimer.stop();
        m_activeExtraction->result = result;

        QEventLoop *loop = m_activeExtraction->loop;
        m_activeExtraction = nullptr;

        if (m_pipe->isRunning()) {
            m_idleShutdownTimer.start(kExtractorIdleShutdownMs);
        }

        if (loop && loop->isRunning()) {
            loop->quit();
        }
    }

    void stopPipe()
    {
        Q_ASSERT(QThread::currentThread() == thread());

        if (!m_pipe->isRunning()) {
            return;
        }

        m_stoppingPipe = true;
        fmInfo() << "ProcessExtractorProxy: stopping extractor process";
        m_pipe->stop();
        m_stoppingPipe = false;
    }

private:
    const QString m_extractorPath;
    EXTRACTOR_NAMESPACE::ControllerPipe *m_pipe { nullptr };
    ActiveExtraction *m_activeExtraction { nullptr };
    QTimer m_requestTimeoutTimer;
    QTimer m_idleShutdownTimer;
    bool m_stoppingPipe { false };
};

}   // namespace

class ProcessExtractorPrivate
{
public:
    ProcessExtractorPrivate()
        : proxy(new ProcessExtractorProxy(QStringLiteral(DFM_EXTRACTOR_TOOL)))
    {
    }

    ~ProcessExtractorPrivate()
    {
        delete proxy;
    }

    ProcessExtractorProxy *proxy { nullptr };
};

IndexExtractionResult invokeProxyExtract(ProcessExtractorProxy *proxy, const QString &filePath, size_t maxBytes)
{
    IndexExtractionResult result;
    const bool invoked = QMetaObject::invokeMethod(
            proxy, [proxy, &result, filePath, maxBytes]() {
                result = proxy->extractSync(filePath, maxBytes);
            },
            Qt::BlockingQueuedConnection);

    if (!invoked) {
        fmCritical() << "ProcessExtractor::extract: failed to invoke extractor request for:" << filePath;
        return { false, QString(), QStringLiteral("Failed to invoke extractor request") };
    }

    return result;
}

ProcessExtractor::ProcessExtractor()
    : d(new ProcessExtractorPrivate())
{
}

ProcessExtractor::~ProcessExtractor()
{
    if (d->proxy) {
        Q_ASSERT(QThread::currentThread() == d->proxy->thread());
        d->proxy->shutdown();
    }
}

IndexExtractionResult ProcessExtractor::extract(const QString &filePath, size_t maxBytes) const
{
    if (!d->proxy) {
        fmCritical() << "ProcessExtractor::extract: extractor proxy is unavailable for:" << filePath;
        return { false, QString(), QStringLiteral("Extractor proxy is unavailable") };
    }

    if (QThread::currentThread() == d->proxy->thread()) {
        return d->proxy->extractSync(filePath, maxBytes);
    }

    return invokeProxyExtract(d->proxy, filePath, maxBytes);
}

SERVICETEXTINDEX_END_NAMESPACE
