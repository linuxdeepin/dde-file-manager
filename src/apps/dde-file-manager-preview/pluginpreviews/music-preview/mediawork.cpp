// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mediawork.h"

using namespace plugin_filepreview;

MediaWork::MediaWork(QObject *parent)
    : QObject(parent)
{
}

void MediaWork::createMediaPlayer()
{
    mediaPlayer = new QMediaPlayer;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    connect(mediaPlayer, &QMediaPlayer::stateChanged, this, &MediaWork::playerStateChanged);
#else
    // Qt6需要显式设置音频输出
    audioOutput = new QAudioOutput(this);
    mediaPlayer->setAudioOutput(audioOutput);
    connect(mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &MediaWork::playerStateChanged);
#endif
    connect(mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &MediaWork::playerStatusChanged);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &MediaWork::playerDurationChanged);
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &MediaWork::playerPositionChanged);
}

void MediaWork::setMedia(const QUrl &url)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (mediaPlayer)
        mediaPlayer->setMedia(url);
#else
    if (mediaPlayer)
        mediaPlayer->setSource(url);
#endif
}

void MediaWork::setPosition(qint64 pos)
{
    if (mediaPlayer)
        mediaPlayer->setPosition(pos);
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
QMediaPlayer::State MediaWork::state() const
{
    if (mediaPlayer)
        return mediaPlayer->state();
    return QMediaPlayer::State::StoppedState;
}
#else
QMediaPlayer::PlaybackState MediaWork::state() const
{
    if (mediaPlayer)
        return mediaPlayer->playbackState();
    return QMediaPlayer::PlaybackState::StoppedState;
}
#endif

qint64 MediaWork::position() const
{
    if (mediaPlayer)
        return mediaPlayer->position();
    return -1;
}

void MediaWork::play()
{
    if (mediaPlayer)
        mediaPlayer->play();
}

void MediaWork::pause()
{
    if (mediaPlayer)
        mediaPlayer->pause();
}

void MediaWork::stop()
{
    if (mediaPlayer)
        mediaPlayer->stop();
}

qint64 MediaWork::duration() const
{
    if (mediaPlayer)
        return mediaPlayer->duration();
    return -1;
}
