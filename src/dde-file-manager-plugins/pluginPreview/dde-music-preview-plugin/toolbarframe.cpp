/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "toolbarframe.h"

#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QUrl>
#include <QTimer>
#include <QtMath>

ToolBarFrame::ToolBarFrame(const QString &uri, QWidget *parent) :
    QFrame(parent)
{
    m_player = new QMediaPlayer(this);

    m_updateProgressTimer = new QTimer(this);
    m_updateProgressTimer->setInterval(1000);

    initUI();
    initConnections();

    m_player->setMedia(QUrl::fromUserInput(uri));
}

void ToolBarFrame::initUI()
{
    m_playControlButton = new QPushButton(this);
    m_playControlButton->setFixedSize(24, 24);
    m_playControlButton->setStyleSheet("QPushButton{"
                                       "border: none;"
                                       "image: url(:/icons/icons/start_normal.png);"
                                       "}"
                                       "QPushButton::pressed{"
                                       "image: url(:/icons/icons/start_pressed.png);"
                                       "}"
                                       "QPushButton::hover{"
                                       "image: url(:/icons/icons/start_hover.png);"
                                       "}"
                                      );

    m_progressSlider = new QSlider(this);
    m_progressSlider->setOrientation(Qt::Horizontal);
    m_progressSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_durationLabel = new QLabel(this);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_playControlButton, 0, Qt::AlignVCenter);
    layout->addWidget(m_progressSlider, 0, Qt::AlignVCenter);
    layout->addWidget(m_durationLabel, 0, Qt::AlignVCenter);

    setLayout(layout);
}

void ToolBarFrame::initConnections()
{
    connect(m_player, &QMediaPlayer::stateChanged, this, &ToolBarFrame::onPlayStateChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &ToolBarFrame::onPlayStatusChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &ToolBarFrame::onPlayDurationChanged);
    connect(m_playControlButton, &QPushButton::clicked, this, &ToolBarFrame::onPlayControlButtonClicked);
    connect(m_updateProgressTimer, &QTimer::timeout, this, &ToolBarFrame::updateProgress);
    connect(m_progressSlider, &QSlider::valueChanged, this, &ToolBarFrame::seekPosition);
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

    QString iconName;
    if (state == QMediaPlayer::StoppedState || state == QMediaPlayer::PausedState) {
        iconName = "start";
    } else {
        iconName = "pause";
    }

    m_playControlButton->setStyleSheet("QPushButton{"
                                       "border: none;"
                                       "image: url(:/icons/icons/" + iconName + "_normal.png);"
                                       "}"
                                       "QPushButton::pressed{"
                                       "image: url(:/icons/icons/" + iconName + "_pressed.png);"
                                       "}"
                                       "QPushButton::hover{"
                                       "image: url(:/icons/icons/" + iconName + "_hover.png);"
                                       "}"
                                      );
}

void ToolBarFrame::onPlayStatusChanged(const QMediaPlayer::MediaStatus &status)
{
    if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia) {
        durationToLabel(m_player->duration());
    }
}

void ToolBarFrame::onPlayControlButtonClicked()
{
    if (m_player->state() == QMediaPlayer::PlayingState) {
        pause();
    } else if (m_player->state() == QMediaPlayer::StoppedState) {
        m_progressSlider->setValue(0);
        play();
    } else {
        play();
    }
}

void ToolBarFrame::updateProgress()
{
    m_progressSlider->setValue(static_cast<int>(m_player->position()));
}

void ToolBarFrame::seekPosition(const int &pos)
{
    if (qAbs(pos - m_player->position()) > 3) {
        m_player->setPosition(pos);
    }
}

void ToolBarFrame::play()
{
    m_player->play();
    m_updateProgressTimer->start();
}

void ToolBarFrame::pause()
{
    m_player->pause();
    m_updateProgressTimer->stop();
}

void ToolBarFrame::stop()
{
    m_progressSlider->setValue(0);
    m_player->stop();
    m_updateProgressTimer->stop();
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

    m_durationLabel->setText(QString("%1: %2").arg(minutesStr, secondsStr));

    m_progressSlider->setMinimum(0);
    m_progressSlider->setMaximum(static_cast<int>(duration));
}

