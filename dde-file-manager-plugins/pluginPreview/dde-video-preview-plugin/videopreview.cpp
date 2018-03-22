/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#include "videopreview.h"

#include <player_widget.h>
#include <player_engine.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/avutil.h>
}

#include <danchors.h>

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class VideoWidget : public dmr::PlayerWidget
{
public:
    VideoWidget(VideoPreview *preview)
        : dmr::PlayerWidget(0)
        , p(preview)
        , title(new QLabel(this))
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        QPalette pa;
        pa.setColor(QPalette::Foreground, Qt::white);

        title->setPalette(pa);

        DAnchorsBase::setAnchor(title, Qt::AnchorHorizontalCenter, this, Qt::AnchorHorizontalCenter);
    }

    QSize sizeHint() const override
    {
        QSize screen_size;

        if (window()->windowHandle()) {
            screen_size = window()->windowHandle()->screen()->availableSize();
        } else {
            screen_size = qApp->desktop()->size();
        }

        return QSize(p->info.width, p->info.height).scaled(qMin(p->info.width, int(screen_size.width() * 0.5)),
                                                           qMin(p->info.height, int(screen_size.height() * 0.5)), Qt::KeepAspectRatio);
    }

    VideoPreview *p;
    QLabel *title;
};

class VideoStatusBar : public QWidget
{
public:
    VideoStatusBar(VideoPreview *preview)
        : QWidget(0)
        , p(preview)
        , slider(new QSlider(this))
        , timeLabel(new QLabel(this))
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QPalette pa_label;

        pa_label.setColor(QPalette::Foreground, QColor("#303030"));
        timeLabel->setPalette(pa_label);

        slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        slider->setMinimum(0);
        slider->setOrientation(Qt::Horizontal);

        QHBoxLayout *layout = new QHBoxLayout(this);

        layout->setContentsMargins(20, 0, 20, 0);
        layout->addWidget(slider);
        layout->addWidget(timeLabel);

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
                slider->setValue(p->playerWidget->engine().elapsed());
            }
            timeLabel->setText(dmr::utils::Time2str(p->playerWidget->engine().elapsed()));
        });
    }

    VideoPreview *p;
    QSlider *slider;
    QLabel *timeLabel;
    bool sliderIsPressed = false;
};

VideoPreview::VideoPreview(QObject *parent)
    : DFMFilePreview(parent)
{
    setlocale(LC_NUMERIC, "C");

    playerWidget = new VideoWidget(this);
    statusBar = new VideoStatusBar(this);
}

VideoPreview::~VideoPreview()
{
    if (playerWidget) {
        playerWidget->setParent(0);
        delete playerWidget.data();
    }

    if (statusBar) {
        statusBar->setParent(0);
        delete statusBar.data();
    }
}

bool VideoPreview::setFileUrl(const DUrl &url)
{
    if (!playerWidget->engine().isPlayableFile(url))
        return false;

    bool ok = false;
    info = dmr::MovieInfo::parseFromFile(QFileInfo(url.toLocalFile()), &ok);

    if (!ok) {
        return false;
    }

    playerWidget->title->setText(info.title);
    playerWidget->title->adjustSize();
    statusBar->slider->setMaximum(info.duration);
    videoUrl = QUrl(url.toLocalFile());

    return true;
}

QWidget *VideoPreview::contentWidget() const
{
    return playerWidget.data();
}

QWidget *VideoPreview::statusBarWidget() const
{
    return statusBar.data();
}

bool VideoPreview::showStatusBarSeparator() const
{
    return false;
}

Qt::Alignment VideoPreview::statusBarWidgetAlignment() const
{
    return Qt::Alignment();
}

void VideoPreview::play()
{
    playerWidget->play(videoUrl);
}

void VideoPreview::pause()
{
    playerWidget->engine().pauseResume();
}

void VideoPreview::stop()
{
    playerWidget->engine().stop();
}

DFM_END_NAMESPACE
