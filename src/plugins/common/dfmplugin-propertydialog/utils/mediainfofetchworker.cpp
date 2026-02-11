// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mediainfofetchworker.h"

#include <QProcess>
#include <QStandardPaths>
#include <QRegularExpression>

DPPROPERTYDIALOG_USE_NAMESPACE

MediaInfoFetchWorker::MediaInfoFetchWorker(QObject *parent)
    : QObject(parent)
{
}

void MediaInfoFetchWorker::getDuration(const QString &filePath)
{
    if (!hasFFmpeg())
        return;

    QProcess ffmpeg;
    ffmpeg.start("ffmpeg", {"-i", filePath});
    bool finished = ffmpeg.waitForFinished(5000); // 5秒超时
    if (!finished) {
        Q_EMIT durationReady("");
        return;
    }

    QByteArray output = ffmpeg.readAllStandardError();
    QRegularExpression re("Duration:\\s+(\\d+:\\d+:\\d+)");
    QRegularExpressionMatch match = re.match(output);

    if (!match.hasMatch())
        return;

    QString duration = match.captured(1);
    Q_EMIT durationReady(duration);
}

bool MediaInfoFetchWorker::hasFFmpeg()
{
    QString ffmpegPath = QStandardPaths::findExecutable("ffmpeg");
    return !ffmpegPath.isEmpty();
}
