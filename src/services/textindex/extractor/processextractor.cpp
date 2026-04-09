// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processextractor.h"
#include "config.h"

#include <controllerpipe.h>

#include <QEventLoop>
#include <QStandardPaths>
#include <QTimer>

SERVICETEXTINDEX_BEGIN_NAMESPACE

IndexExtractionResult ProcessExtractor::extract(const QString &filePath, size_t maxBytes) const
{
    Q_UNUSED(maxBytes)

    IndexExtractionResult result;
    const QString extractorPath(DFM_EXTRACTOR_TOOL);
    if (extractorPath.isEmpty()) {
        result.error = QStringLiteral("dde-file-manager-extractor not found");
        return result;
    }

    EXTRACTOR_NAMESPACE::ControllerPipe pipe;
    if (!pipe.start(extractorPath)) {
        result.error = QStringLiteral("Failed to start dde-file-manager-extractor");
        return result;
    }

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, [&]() {
        if (!result.success && result.error.isEmpty()) {
            result.error = QStringLiteral("Extractor request timed out");
        }
        loop.quit();
    });

    QObject::connect(&pipe, &EXTRACTOR_NAMESPACE::ControllerPipe::extractionFinished, &loop,
                     [&](const QString &path, const QByteArray &data) {
                         if (path != filePath) {
                             return;
                         }

                         result.success = true;
                         result.text = QString::fromUtf8(data).trimmed();
                         loop.quit();
                     });

    QObject::connect(&pipe, &EXTRACTOR_NAMESPACE::ControllerPipe::extractionFailed, &loop,
                     [&](const QString &path, const QString &error) {
                         if (path != filePath) {
                             return;
                         }

                         result.error = error;
                         loop.quit();
                     });

    QObject::connect(&pipe, &EXTRACTOR_NAMESPACE::ControllerPipe::errorOccurred, &loop,
                     [&](const QString &error) {
                         if (result.error.isEmpty()) {
                             result.error = error;
                         }
                         loop.quit();
                     });

    QObject::connect(&pipe, &EXTRACTOR_NAMESPACE::ControllerPipe::processCrashed, &loop, [&]() {
        if (result.error.isEmpty()) {
            result.error = QStringLiteral("Extractor process crashed");
        }
        loop.quit();
    });

    if (!pipe.extractBatch({ filePath })) {
        pipe.stop();
        result.error = QStringLiteral("Failed to send extractor request");
        return result;
    }

    timeoutTimer.start(30000);
    loop.exec();
    pipe.stop();

    return result;
}

SERVICETEXTINDEX_END_NAMESPACE
