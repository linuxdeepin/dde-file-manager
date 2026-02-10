// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSMEDIAPLAYER_H
#define CUSMEDIAPLAYER_H

#include <QObject>
#include <QThread>
#include <QMediaPlayer>

namespace plugin_filepreview {
class MediaWork;

class CusMediaPlayer : public QObject
{
    Q_OBJECT
public:
    static CusMediaPlayer *instance();
    ~CusMediaPlayer();

    void createMediaPlayer();
    void setMedia(const QUrl &url);

    qint64 duration() const;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QMediaPlayer::State state() const;
#else
    QMediaPlayer::PlaybackState state() const;
#endif
    qint64 position() const;

Q_SIGNALS:
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    void sigStateChanged(QMediaPlayer::State newState);
#else
    void sigStateChanged(QMediaPlayer::PlaybackState newState);
#endif
    void sigStatusChanged(QMediaPlayer::MediaStatus status);
    void sigDurationChanged(qint64 duration);
    void sigPositionChanged(qint64 pos);

    void sigCreateMediaPlayer();
    void sigSetMedia(const QUrl &url);
    void sigSetPosition(qint64 pos);

    void sigPlay();
    void sigPause();
    void sigStop();

private:
    explicit CusMediaPlayer(QObject *parent = nullptr);

    QThread workerThread;
    MediaWork *worker { Q_NULLPTR };
    bool isFirst { true };
};
}

#endif   // CUSMEDIAPLAYER_H
