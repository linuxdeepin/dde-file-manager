// SPDX-FileCopyrightText: 2022 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEDIAWORK_H
#define MEDIAWORK_H

#include <QObject>
#include <QMediaPlayer>

namespace plugin_filepreview {
class MediaWork : public QObject
{
    Q_OBJECT
public:
    explicit MediaWork(QObject *parent = nullptr);
    qint64 duration() const;
    QMediaPlayer::State state() const;
    qint64 position() const;

public Q_SLOTS:
    void createMediaPlayer();
    void setMedia(const QUrl &url);
    void setPosition(qint64 pos);
    void play();
    void pause();
    void stop();

Q_SIGNALS:
    void playerStateChanged(QMediaPlayer::State newState);
    void playerStatusChanged(QMediaPlayer::MediaStatus status);
    void playerDurationChanged(qint64 duration);
    void playerPositionChanged(qint64 duration);

private:
    QMediaPlayer *mediaPlayer;
};
}

#endif // MEDIAWORK_H
