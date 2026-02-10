// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEDIAWORK_H
#define MEDIAWORK_H

#include <QObject>
#include <QMediaPlayer>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QAudioOutput>
#endif

namespace plugin_filepreview {
class MediaWork : public QObject
{
    Q_OBJECT
public:
    explicit MediaWork(QObject *parent = nullptr);
    qint64 duration() const;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QMediaPlayer::State state() const;
#else
    QMediaPlayer::PlaybackState state() const;
#endif
    qint64 position() const;

public Q_SLOTS:
    void createMediaPlayer();
    void setMedia(const QUrl &url);
    void setPosition(qint64 pos);
    void play();
    void pause();
    void stop();

Q_SIGNALS:
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    void playerStateChanged(QMediaPlayer::State newState);
#else
    void playerStateChanged(QMediaPlayer::PlaybackState newState);
#endif
    void playerStatusChanged(QMediaPlayer::MediaStatus status);
    void playerDurationChanged(qint64 duration);
    void playerPositionChanged(qint64 duration);

private:
    QMediaPlayer *mediaPlayer { nullptr };
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QAudioOutput *audioOutput { nullptr };
#endif
};
}

#endif   // MEDIAWORK_H
