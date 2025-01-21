// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
    setlocale(LC_NUMERIC, "C");

    playerWidget = new VideoWidget(this);
    playerWidget->setMinimumSize(800, 355);
    statusBar = new VideoStatusBar(this);

    connect(&playerWidget->engine(), &dmr::PlayerEngine::stateChanged, this, &VideoPreview::sigPlayState);
    connect(&playerWidget->engine(), &dmr::PlayerEngine::elapsedChanged, this, &VideoPreview::elapsedChanged);
}

VideoPreview::~VideoPreview()
{
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
}

bool VideoPreview::setFileUrl(const QUrl &url)
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
    videoUrl = QUrl::fromLocalFile(url.toLocalFile());

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
        playerWidget->playFile(videoUrl);
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
