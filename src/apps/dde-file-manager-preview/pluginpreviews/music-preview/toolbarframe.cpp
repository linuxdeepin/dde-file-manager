// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbarframe.h"
#include "cusmediaplayer.h"

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
    initUI();
    initConnections();

    CusMediaPlayer::instance()->createMediaPlayer();
    CusMediaPlayer::instance()->setMedia(QUrl::fromUserInput(uri));
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
    connect(CusMediaPlayer::instance(), &CusMediaPlayer::sigStateChanged, this, &ToolBarFrame::onPlayStateChanged);
    connect(CusMediaPlayer::instance(), &CusMediaPlayer::sigStatusChanged, this, &ToolBarFrame::onPlayStatusChanged);
    connect(CusMediaPlayer::instance(), &CusMediaPlayer::sigDurationChanged, this, &ToolBarFrame::onPlayDurationChanged);
    connect(CusMediaPlayer::instance(), &CusMediaPlayer::sigPositionChanged, this, &ToolBarFrame::onPlayPositionChanged);
    connect(playControlButton, &QPushButton::clicked, this, &ToolBarFrame::onPlayControlButtonClicked);
    connect(progressSlider, &DSlider::valueChanged, this, &ToolBarFrame::seekPosition);
}

void ToolBarFrame::onPlayDurationChanged(qint64 duration)
{
    durationToLabel(duration);
}

void ToolBarFrame::onPlayPositionChanged(qint64 pos)
{
    if (pos == lastPos || curState == QMediaPlayer::StoppedState)
        return;
    lastPos = pos;
    progressSlider->setValue(static_cast<int>(pos));
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
void ToolBarFrame::onPlayStateChanged(const QMediaPlayer::State &state)
#else
void ToolBarFrame::onPlayStateChanged(const QMediaPlayer::PlaybackState &state)
#endif
{
    curState = state;
    if (state == QMediaPlayer::StoppedState)
        progressSlider->setValue(0);

    if (state == QMediaPlayer::StoppedState || state == QMediaPlayer::PausedState) {
        playControlButton->setIcon(QIcon::fromTheme(":/icons/icons/start_normal.png"));
    } else {
        playControlButton->setIcon(QIcon::fromTheme(":/icons/icons/pause_normal.png"));
    }
}

void ToolBarFrame::onPlayStatusChanged(const QMediaPlayer::MediaStatus &status)
{
    if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia)
        durationToLabel(CusMediaPlayer::instance()->duration());
}

void ToolBarFrame::onPlayControlButtonClicked()
{
    auto sta = CusMediaPlayer::instance()->state();
    if (sta == QMediaPlayer::PlayingState) {
        pause();
    } else if (sta == QMediaPlayer::StoppedState) {
        progressSlider->setValue(0);
        play();
    } else {
        play();
    }
}

void ToolBarFrame::seekPosition(const int &pos)
{
    if (qAbs(pos - CusMediaPlayer::instance()->position()) > 3) {
        emit CusMediaPlayer::instance()->sigSetPosition(pos);
    }
}

void ToolBarFrame::play()
{
    curState = QMediaPlayer::PlayingState;
    emit CusMediaPlayer::instance()->sigPlay();
}

void ToolBarFrame::pause()
{
    curState = QMediaPlayer::PausedState;
    emit CusMediaPlayer::instance()->sigPause();
}

void ToolBarFrame::stop()
{
    curState = QMediaPlayer::StoppedState;
    progressSlider->setValue(0);
    emit CusMediaPlayer::instance()->sigStop();
}

void ToolBarFrame::durationToLabel(qint64 duration)
{
    if (duration < 1) {
        durationLabel->setText("--");
        progressSlider->setMinimum(0);
        progressSlider->setMaximum(0);
        return;
    }

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
