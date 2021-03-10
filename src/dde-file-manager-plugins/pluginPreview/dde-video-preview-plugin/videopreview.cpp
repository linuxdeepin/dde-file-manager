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

#include "videopreview.h"

#include <player_widget.h>
#include <player_engine.h>
#include <compositing_manager.h>

#include <danchors.h>
#include <dimagebutton.h>

//DWIDGET_USE_NAMESPACE

//DFM_USE_NAMESPACE

//class VideoWidget : public dmr::PlayerWidget
//{
//public:
//     VideoWidget(VideoPreview *preview)
//        : dmr::PlayerWidget(nullptr)
//        , p(preview)
//        , title(new QLabel(this))
//    {
//        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

//        QPalette pa;
//        pa.setColor(QPalette::Foreground, Qt::white);

//        title->setPalette(pa);

//        DAnchorsBase::setAnchor(title, Qt::AnchorHorizontalCenter, this, Qt::AnchorHorizontalCenter);

//        engine().setBackendProperty("keep-open", "yes");
//    }

//    QSize sizeHint() const override
//    {
//        QSize screen_size;

//        if (window()->windowHandle()) {
//            screen_size = window()->windowHandle()->screen()->availableSize();
//        } else {
//            screen_size = qApp->desktop()->size();
//        }

//        return QSize(p->info.width, p->info.height).scaled(qMin(p->info.width, int(screen_size.width() * 0.5)),
//                                                           qMin(p->info.height, int(screen_size.height() * 0.5)), Qt::KeepAspectRatio);
//    }

//    void mouseReleaseEvent(QMouseEvent *event) override
//    {
//        p->pause();

//        dmr::PlayerWidget::mouseReleaseEvent(event);
//    }

//    VideoPreview *p;
//    QLabel *title;
//};

//class VideoStatusBar : public QWidget
//{
//public:
//    explicit VideoStatusBar(VideoPreview *preview)
//        : QWidget(nullptr)
//        , p(preview)
//        , slider(new QSlider(this))
//        , timeLabel(new QLabel(this))
//    {
//        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

//        DImageButton *control_button = new DImageButton(this);
////        QWidget *button_mask = new QWidget(control_button);

//        control_button->setNormalPic(":/icons/icons/start_normal.png");
//        control_button->setPressPic(":/icons/icons/start_pressed.png");
//        control_button->setHoverPic(":/icons/icons/start_hover.png");
////        button_mask->setAutoFillBackground(true);

////        DAnchorsBase(button_mask).setFill(control_button);

//        QPalette pa_label;

//        pa_label.setColor(QPalette::Foreground, QColor("#303030"));
//        timeLabel->setPalette(pa_label);

//        slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//        slider->setMinimum(0);
//        slider->setOrientation(Qt::Horizontal);

//        QHBoxLayout *layout = new QHBoxLayout(this);

//        layout->setContentsMargins(0, 0, 20, 0);
//        layout->addWidget(control_button);
//        layout->addWidget(slider);
//        layout->addWidget(timeLabel);

//        connect(control_button, &DImageButton::clicked, this, [this] {
//            // 由于调用了setBackendProperty("keep-open", "yes")
//            // 导致视频播放状态不对（要暂停到最后一帧，所以视频播放完毕后状态还是暂停）
//            // 如果是暂停状态，调用pause一定会播放，之后再调用play也没有影响
//            // 反之，如果是停止状态，调用pause无反应，之后再调用play肯定会播放
//            p->pause();
//            p->playerWidget->engine().play();

////            button_mask->show();
//        });
//        connect(&p->playerWidget->engine(), &dmr::PlayerEngine::stateChanged, this, [this, control_button] {
//            if (p->playerWidget->engine().state() == dmr::PlayerEngine::Playing)
//            {
////                button_mask->show();
//                control_button->setNormalPic(":/icons/icons/pause_normal.png");
//                control_button->setPressPic(":/icons/icons/pause_pressed.png");
//                control_button->setHoverPic(":/icons/icons/pause_hover.png");
//            } else
//            {
//                control_button->setNormalPic(":/icons/icons/start_normal.png");
//                control_button->setPressPic(":/icons/icons/start_pressed.png");
//                control_button->setHoverPic(":/icons/icons/start_hover.png");
////                button_mask->hide();
//            }
//        });
//        connect(slider, &QSlider::valueChanged, this, [this] {
//            p->playerWidget->engine().seekAbsolute(slider->value());
//        });
//        connect(slider, &QSlider::sliderPressed, this, [this] {
//            sliderIsPressed = true;
//        });
//        connect(slider, &QSlider::sliderReleased, this, [this] {
//            sliderIsPressed = false;
//        });
//        connect(&p->playerWidget->engine(), &dmr::PlayerEngine::elapsedChanged, this, [this] {
//            if (!sliderIsPressed)
//            {
//                QSignalBlocker blocker(slider);
//                Q_UNUSED(blocker)
//                slider->setValue(static_cast<int>(p->playerWidget->engine().elapsed()));
//            }
//            timeLabel->setText(dmr::utils::Time2str(p->playerWidget->engine().elapsed()));
//        });
//    }

//    VideoPreview *p;
//    QSlider *slider;
//    QLabel *timeLabel;
//    bool sliderIsPressed = false;
//};

VideoPreview::VideoPreview(QObject *parent)
    : DFMFilePreview(parent)
{
    setlocale(LC_NUMERIC, "C");

    // 强制不使用嵌入mpv窗口的模式
    // 经确认，该函数已废弃，屏蔽后不影响使用
 //   dmr::CompositingManager::get().overrideCompositeMode(true);

    playerWidget = new VideoWidget(this);
    statusBar = new VideoStatusBar(this);
}

VideoPreview::~VideoPreview()
{
    if (playerWidget) {
        playerWidget->hide();
        delete playerWidget.data();
    }

    if (statusBar) {
        statusBar->hide();
        statusBar->deleteLater();
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
    statusBar->slider->setMaximum(static_cast<int>(info.duration));
    videoUrl = QUrl(url.toLocalFile());

    return true;
}

DUrl VideoPreview::fileUrl() const
{
    return DUrl(videoUrl);
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

void VideoPreview::DoneCurrent()
{
    //非wayland平台请注释掉这段代码
#ifdef __armWayland__
    playerWidget->DoneCurrent();
#endif
}

void VideoPreview::play()
{
    if (playerWidget) {
        playerWidget->play(videoUrl);
    }
}

void VideoPreview::pause()
{
    playerWidget->engine().pauseResume();
}

void VideoPreview::stop()
{
    playerWidget->engine().stop();
}
