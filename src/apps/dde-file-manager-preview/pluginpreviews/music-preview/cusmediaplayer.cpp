// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cusmediaplayer.h"
#include "mediawork.h"

using namespace plugin_filepreview;

CusMediaPlayer *CusMediaPlayer::instance()
{
    static CusMediaPlayer ins;
    return &ins;
}

CusMediaPlayer::~CusMediaPlayer()
{
    workerThread.quit();
    workerThread.wait();
}

void CusMediaPlayer::createMediaPlayer()
{
    if (isFirst) {
        emit sigCreateMediaPlayer();
        isFirst = false;
    }
}

void CusMediaPlayer::setMedia(const QUrl &url)
{
    emit sigStop();
    emit sigSetMedia(url);
}

qint64 CusMediaPlayer::duration() const
{
    return worker->duration();
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
QMediaPlayer::State CusMediaPlayer::state() const
#else
QMediaPlayer::PlaybackState CusMediaPlayer::state() const
#endif
{
    return worker->state();
}

qint64 CusMediaPlayer::position() const
{
    return worker->position();
}

CusMediaPlayer::CusMediaPlayer(QObject *parent)
    : QObject(parent)
{
    worker = new MediaWork;
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &CusMediaPlayer::sigCreateMediaPlayer, worker, &MediaWork::createMediaPlayer);
    connect(this, &CusMediaPlayer::sigSetMedia, worker, &MediaWork::setMedia);
    connect(this, &CusMediaPlayer::sigPlay, worker, &MediaWork::play);
    connect(this, &CusMediaPlayer::sigPause, worker, &MediaWork::pause);
    connect(this, &CusMediaPlayer::sigStop, worker, &MediaWork::stop);
    connect(this, &CusMediaPlayer::sigSetPosition, worker, &MediaWork::setPosition);
    connect(worker, &MediaWork::playerStateChanged, this, &CusMediaPlayer::sigStateChanged);
    connect(worker, &MediaWork::playerStatusChanged, this, &CusMediaPlayer::sigStatusChanged);
    connect(worker, &MediaWork::playerDurationChanged, this, &CusMediaPlayer::sigDurationChanged);
    connect(worker, &MediaWork::playerPositionChanged, this, &CusMediaPlayer::sigPositionChanged);
    workerThread.start();
}
