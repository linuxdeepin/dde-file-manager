// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videostatusbar.h"
#include "videopreview.h"

#include <DIconButton>

DWIDGET_USE_NAMESPACE
DFM_USE_NAMESPACE
VideoStatusBar::VideoStatusBar(VideoPreview *preview)
    : QWidget(nullptr)
    , p(preview)
    , slider(new QSlider(this))
    , timeLabel(new QLabel(this))
    , sliderIsPressed(false)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    DIconButton *control_button = new DIconButton(this);
    control_button->setFlat(true);
    control_button->setIconSize({24, 24});

    control_button->setIcon(QIcon::fromTheme("dfm_pause"));

    QPalette pa_label;

    pa_label.setColor(QPalette::Foreground, QColor("#303030"));
    timeLabel->setPalette(pa_label);

    slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    slider->setMinimum(0);
    slider->setOrientation(Qt::Horizontal);

    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->setContentsMargins(0, 0, 20, 0);
    layout->addWidget(control_button);
    layout->addWidget(slider);
    layout->addWidget(timeLabel);

    connect(control_button, &DIconButton::clicked, this, [this] {
        // 由于调用了setBackendProperty("keep-open", "yes")
        // 导致视频播放状态不对（要暂停到最后一帧，所以视频播放完毕后状态还是暂停）
        // 如果是暂停状态，调用pause一定会播放，之后再调用play也没有影响
        // 反之，如果是停止状态，调用pause无反应，之后再调用play肯定会播放
        p->pause();
        p->playerWidget->engine().play();
    });
    connect(&p->playerWidget->engine(), &dmr::PlayerEngine::stateChanged, this, [this, control_button] {
        if (p->playerWidget->engine().state() == dmr::PlayerEngine::Playing) {
            control_button->setIcon(QIcon::fromTheme("dfm_pause"));
        } else {
            control_button->setIcon(QIcon::fromTheme("dfm_start"));
        }
    });
    connect(slider, &QSlider::valueChanged, this, [this] {
        p->playerWidget->engine().seekAbsolute(slider->value());
    });
    connect(slider, &QSlider::sliderPressed, this, [this] {
        sliderIsPressed = true;
    });
    connect(slider, &QSlider::sliderReleased, this, [this] {
        sliderIsPressed = false;
    });
    connect(&p->playerWidget->engine(), &dmr::PlayerEngine::elapsedChanged, this, [this] {
        if (!sliderIsPressed) {
            QSignalBlocker blocker(slider);
            Q_UNUSED(blocker)
            slider->setValue(static_cast<int>(p->playerWidget->engine().elapsed()));
        }
        timeLabel->setText(dmr::utils::Time2str(p->playerWidget->engine().elapsed()));
    });
}
