// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videowidget.h"
#include "videopreview.h"

#include <player_engine.h>

DWIDGET_USE_NAMESPACE
using namespace plugin_filepreview;
VideoWidget::VideoWidget(VideoPreview *preview)
    : dmr::PlayerWidget(nullptr), p(preview), titleBar(new TitleBarWidget(this))
{
    fmDebug() << "Video widget: initializing VideoWidget with custom title bar";

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Position title bar at the top of the widget
    titleBar->move(0, 0);
    titleBar->setVisible(true);
    titleBar->raise(); // Ensure title bar is above video content

    engine().setBackendProperty("keep-open", "yes");

    fmDebug() << "Video widget: VideoWidget initialization completed";
}

QSize VideoWidget::sizeHint() const
{
    QSize screen_size;

    if (window()->windowHandle()) {
        screen_size = window()->windowHandle()->screen()->availableSize();
    } else {
        screen_size = QGuiApplication::primaryScreen()->availableSize();
    }

    return QSize(p->info.width, p->info.height).scaled(qMin(p->info.width, int(screen_size.width() * 0.5)), qMin(p->info.height, int(screen_size.height() * 0.5)), Qt::KeepAspectRatio);
}

void VideoWidget::playFile(const QUrl &url)
{
    fmDebug() << "Video widget: playFile called with URL:" << url;

    videoUrl = url;

    // If widget is already visible, play immediately to support file switching
    // Otherwise, wait for showEvent to trigger playback (first-time display)
    if (isVisible() && !url.isEmpty()) {
        fmDebug() << "Video widget: widget is visible, playing video immediately";
        play(videoUrl);
    } else {
        fmDebug() << "Video widget: widget not visible, deferring playback to showEvent";
    }
}

void VideoWidget::mouseReleaseEvent(QMouseEvent *event)
{
    p->pause();

    dmr::PlayerWidget::mouseReleaseEvent(event);
}

void VideoWidget::showEvent(QShowEvent *event)
{
    if (!videoUrl.isEmpty())
        play(videoUrl);

    return dmr::PlayerWidget::showEvent(event);
}

void VideoWidget::resizeEvent(QResizeEvent *event)
{
    dmr::PlayerWidget::resizeEvent(event);

    // Update title bar size and position
    if (titleBar) {
        titleBar->setGeometry(0, 0, width(), titleBar->height());
    }
}

void VideoWidget::enterEvent(QEnterEvent *event)
{
    dmr::PlayerWidget::enterEvent(event);

    fmDebug() << "Video widget: mouse entered video widget";

    // When mouse enters the video widget, check if it's in title bar area
    if (titleBar) {
        titleBar->stopAutoHideTimer();
        titleBar->showAnimated();
    }
}

void VideoWidget::leaveEvent(QEvent *event)
{
    dmr::PlayerWidget::leaveEvent(event);

    fmDebug() << "Video widget: mouse left video widget";

    // When mouse leaves the video widget, start auto-hide timer
    if (titleBar) {
        titleBar->hideAnimated();
    }
}

