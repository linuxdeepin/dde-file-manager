// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbarframe.h"

#include <DSlider>

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QUrl>
#include <QTimer>
#include <QtMath>

using namespace plugin_filepreview;
DWIDGET_USE_NAMESPACE
ToolBarFrame::ToolBarFrame(const QString &uri, QWidget *parent)
    : QFrame(parent)
{
    mediaPlayer = new QMediaPlayer(this);

    updateProgressTimer = new QTimer(this);
    updateProgressTimer->setInterval(1000);

    initUI();
    initConnections();

    mediaPlayer->setMedia(QUrl::fromUserInput(uri));
}

void ToolBarFrame::initUI()
{
    playControlButton = new QPushButton(this);
    playControlButton->setFixedSize(36, 36);
    playControlButton->setIcon(QIcon::fromTheme(":/icons/icons/start_normal.png"));

    progressSlider = new DSlider(Qt::Horizontal, this);
    progressSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    durationLabel = new QLabel(this);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(playControlButton, 0, Qt::AlignVCenter);
    layout->addWidget(progressSlider, 0, Qt::AlignVCenter);
    layout->addWidget(durationLabel, 0, Qt::AlignVCenter);
    layout->addSpacing(5);

    setLayout(layout);
}

void ToolBarFrame::initConnections()
{
    connect(mediaPlayer, &QMediaPlayer::stateChanged, this, &ToolBarFrame::onPlayStateChanged);
    connect(mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &ToolBarFrame::onPlayStatusChanged);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &ToolBarFrame::onPlayDurationChanged);
    connect(playControlButton, &QPushButton::clicked, this, &ToolBarFrame::onPlayControlButtonClicked);
    connect(updateProgressTimer, &QTimer::timeout, this, &ToolBarFrame::updateProgress);
    connect(progressSlider, &DSlider::valueChanged, this, &ToolBarFrame::seekPosition);
}

void ToolBarFrame::onPlayDurationChanged(qint64 duration)
{
    durationToLabel(duration);
}

void ToolBarFrame::onPlayStateChanged(const QMediaPlayer::State &state)
{
    if (state == QMediaPlayer::StoppedState) {
        stop();
    }

    if (state == QMediaPlayer::StoppedState || state == QMediaPlayer::PausedState) {
        playControlButton->setIcon(QIcon::fromTheme(":/icons/icons/start_normal.png"));
    } else {
        playControlButton->setIcon(QIcon::fromTheme(":/icons/icons/pause_normal.png"));
    }
}

void ToolBarFrame::onPlayStatusChanged(const QMediaPlayer::MediaStatus &status)
{
    if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia) {
        durationToLabel(mediaPlayer->duration());
    }
}

void ToolBarFrame::onPlayControlButtonClicked()
{
    if (mediaPlayer->state() == QMediaPlayer::PlayingState) {
        pause();
    } else if (mediaPlayer->state() == QMediaPlayer::StoppedState) {
        progressSlider->setValue(0);
        play();
    } else {
        play();
    }
}

void ToolBarFrame::updateProgress()
{
    progressSlider->setValue(static_cast<int>(mediaPlayer->position()));
}

void ToolBarFrame::seekPosition(const int &pos)
{
    if (qAbs(pos - mediaPlayer->position()) > 3) {
        mediaPlayer->setPosition(pos);
    }
}

void ToolBarFrame::play()
{
    mediaPlayer->play();
    updateProgressTimer->start();
}

void ToolBarFrame::pause()
{
    mediaPlayer->pause();
    updateProgressTimer->stop();
}

void ToolBarFrame::stop()
{
    progressSlider->setValue(0);
    mediaPlayer->stop();
    updateProgressTimer->stop();
}

void ToolBarFrame::durationToLabel(qint64 duration)
{
    qlonglong msDuration = duration;
    qlonglong sDurations = msDuration / 1000;
    int minutes = static_cast<int>(sDurations) / 60;
    int seconds = sDurations % 60;
    QString minutesStr;
    QString secondsStr;
    if (minutes < 10) {
        minutesStr = "0" + QString::number(minutes);
    } else {
        minutesStr = QString::number(minutes);
    }
    if (seconds < 10) {
        secondsStr = "0" + QString::number(seconds);
    } else {
        secondsStr = QString::number(seconds);
    }

    durationLabel->setText(QString("%1: %2").arg(minutesStr, secondsStr));

    progressSlider->setMinimum(0);
    progressSlider->setMaximum(static_cast<int>(duration));
}
