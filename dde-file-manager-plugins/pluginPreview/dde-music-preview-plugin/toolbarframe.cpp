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

    QHBoxLayout* layout = new QHBoxLayout;
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
    connect(m_playControlButton, &QPushButton::clicked, this, &ToolBarFrame::onPlayControlButtonClicked);
    connect(m_updateProgressTimer, &QTimer::timeout, this, &ToolBarFrame::updateProgress);
    connect(m_progressSlider, &QSlider::valueChanged, this, &ToolBarFrame::seekPosition);
}

void ToolBarFrame::onPlayStateChanged(const QMediaPlayer::State &state)
{
    QString iconName;
    if(state == QMediaPlayer::StoppedState || state == QMediaPlayer::PausedState){
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
    if(status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia){
        qlonglong msDuration = m_player->duration();
        qlonglong sDurations = msDuration / 1000;
        int minutes = sDurations / 60;
        int seconds = sDurations % 60;
        QString minutesStr;
        QString secondsStr;
        if(minutes < 10){
            minutesStr = "0" + QString::number(minutes);
        } else {
            minutesStr = QString::number(minutes);
        }
        if(seconds < 10){
            secondsStr = "0" + QString::number(seconds);
        } else {
            secondsStr = QString::number(seconds);
        }

        m_durationLabel->setText(QString("%1: %2").arg(minutesStr, secondsStr));

        m_progressSlider->setMinimum(0);
        m_progressSlider->setMaximum(m_player->duration());
    } else{
        m_durationLabel->setText("00: 00");
    }
}

void ToolBarFrame::onPlayControlButtonClicked()
{
    if(m_player->state() == QMediaPlayer::PlayingState){
        m_player->pause();
        m_updateProgressTimer->stop();
    } else if (m_player->state() == QMediaPlayer::StoppedState){
        m_progressSlider->setValue(0);
        m_player->play();
        m_updateProgressTimer->start();
    } else {
        m_player->play();
        m_updateProgressTimer->start();
    }
}

void ToolBarFrame::updateProgress()
{
    m_progressSlider->setValue(m_player->position());
}

void ToolBarFrame::seekPosition(const int &pos)
{
    if(qAbs(pos - m_player->position()) > 3){
        m_player->setPosition(pos);
    }
}

