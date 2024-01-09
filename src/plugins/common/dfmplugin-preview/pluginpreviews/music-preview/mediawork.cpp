// SPDX-FileCopyrightText: 2022 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mediawork.h"

using namespace plugin_filepreview;

MediaWork::MediaWork(QObject *parent) : QObject(parent)
{

}

void MediaWork::createMediaPlayer()
{
    mediaPlayer = new QMediaPlayer;
    connect(mediaPlayer, &QMediaPlayer::stateChanged, this, &MediaWork::playerStateChanged);
    connect(mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &MediaWork::playerStatusChanged);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &MediaWork::playerDurationChanged);
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &MediaWork::playerPositionChanged);
}

void MediaWork::setMedia(const QUrl &url)
{
    if (mediaPlayer)
        mediaPlayer->setMedia(url);
}

void MediaWork::setPosition(qint64 pos)
{
    if (mediaPlayer)
        mediaPlayer->setPosition(pos);
}

QMediaPlayer::State MediaWork::state() const
{
    if (mediaPlayer)
        return mediaPlayer->state();
    return QMediaPlayer::State::StoppedState;
}

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
