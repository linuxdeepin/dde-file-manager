// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videopreview.h"

#include <danchors.h>
#include <dimagebutton.h>

#include <player_widget.h>
#include <player_engine.h>
#include <compositing_manager.h>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace plugin_filepreview;

VideoPreview::VideoPreview(QObject *parent)
    : AbstractBasePreview(parent)
{
    fmInfo() << "Video preview: VideoPreview instance created";

    setlocale(LC_NUMERIC, "C");

    playerWidget = new VideoWidget(this);
    playerWidget->setMinimumSize(800, 355);
    statusBar = new VideoStatusBar(this);

    connect(&playerWidget->engine(), &dmr::PlayerEngine::stateChanged, this, &VideoPreview::sigPlayState);
    connect(&playerWidget->engine(), &dmr::PlayerEngine::elapsedChanged, this, &VideoPreview::elapsedChanged);

    fmDebug() << "Video preview: VideoPreview initialization completed";
}

VideoPreview::~VideoPreview()
{
    fmInfo() << "Video preview: VideoPreview instance destroyed";

    if (statusBar) {
        statusBar->hide();
        statusBar->deleteLater();
    }

    if (!playerWidget.isNull()) {
        playerWidget->hide();
        disconnect(&playerWidget->engine(), &dmr::PlayerEngine::stateChanged, this, &VideoPreview::sigPlayState);
        disconnect(&playerWidget->engine(), &dmr::PlayerEngine::elapsedChanged, this, &VideoPreview::elapsedChanged);
        playerWidget.data()->deleteLater();
    }

    fmDebug() << "Video preview: VideoPreview cleanup completed";
}

bool VideoPreview::setFileUrl(const QUrl &url)
{
    fmInfo() << "Video preview: setting file URL:" << url;

    if (!url.isLocalFile()) {
        fmWarning() << "Video preview: URL is not a local file:" << url;
        return false;
    }

    const QString filePath = url.toLocalFile();
    if (!QFileInfo::exists(filePath)) {
        fmWarning() << "Video preview: file does not exist:" << filePath;
        return false;
    }

    if (!playerWidget->engine().isPlayableFile(url)) {
        fmWarning() << "Video preview: file is not playable:" << url;
        return false;
    }

    bool ok = false;
    info = dmr::MovieInfo::parseFromFile(QFileInfo(filePath), &ok);

    if (!ok) {
        fmWarning() << "Video preview: failed to parse movie info from file:" << filePath;
        return false;
    }

    fmDebug() << "Video preview: movie info parsed successfully - title:" << info.title << "duration:" << info.duration;

    playerWidget->titleBar->setText(info.title);
    statusBar->slider->setMaximum(static_cast<int>(info.duration));
    videoUrl = QUrl::fromLocalFile(filePath);

    fmInfo() << "Video preview: file URL set successfully:" << url;
    return true;
}

QUrl VideoPreview::fileUrl() const
{
    return QUrl(videoUrl);
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
    if (playerWidget && videoUrl.isValid()) {
        fmDebug() << "Video preview: starting playback for:" << videoUrl;
        playerWidget->playFile(videoUrl);

        // Start the title bar auto-hide timer when playback begins
        if (playerWidget->titleBar) {
            playerWidget->titleBar->startAutoHideTimer();
        }
    } else {
        fmWarning() << "Video preview: cannot play - invalid player widget or URL:" << videoUrl;
    }
}

void VideoPreview::pause()
{
    fmDebug() << "Video preview: pausing/resuming playback";
    playerWidget->engine().pauseResume();
}

void VideoPreview::stop()
{
    fmDebug() << "Video preview: stopping playback";
    playerWidget->engine().stop();
}
