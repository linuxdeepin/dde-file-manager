// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLBARFRAME_H
#define TOOLBARFRAME_H

#include "preview_plugin_global.h"

#include <dtkwidget_global.h>

#include <QFrame>
#include <QMediaPlayer>

QT_BEGIN_NAMESPACE
class QPushButton;
class QSlider;
class QLabel;
class QMediaPlayer;
class QTimer;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DSlider;
DWIDGET_END_NAMESPACE

namespace plugin_filepreview {
class ToolBarFrame : public QFrame
{
    Q_OBJECT
public:
    explicit ToolBarFrame(const QString &uri, QWidget *parent = nullptr);

private:
    void initUI();
    void initConnections();

public slots:
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    void onPlayStateChanged(const QMediaPlayer::State &state);
#else
    void onPlayStateChanged(const QMediaPlayer::PlaybackState &state);

#endif
    void onPlayStatusChanged(const QMediaPlayer::MediaStatus &status);
    void onPlayDurationChanged(qint64 duration);
    void onPlayPositionChanged(qint64 pos);
    void onPlayControlButtonClicked();
    void seekPosition(const int &pos);
    void play();
    void pause();
    void stop();

private:
    void durationToLabel(qint64 duration);

private:
    QPushButton *playControlButton { nullptr };
    DTK_WIDGET_NAMESPACE::DSlider *progressSlider { nullptr };
    QLabel *durationLabel { nullptr };
    QTimer *updateProgressTimer { nullptr };
    qint64 lastPos { -1 };
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QMediaPlayer::State curState { QMediaPlayer::State::StoppedState };
#else
    QMediaPlayer::PlaybackState curState { QMediaPlayer::StoppedState };
#endif
};
}
#endif   // TOOLBARFRAME_H
