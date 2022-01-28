/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#include <danchors.h>
#include <dimagebutton.h>

#include <player_widget.h>
#include <player_engine.h>
#include <compositing_manager.h>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
PREVIEW_USE_NAMESPACE

VideoPreview::VideoPreview(QObject *parent)
    : AbstractBasePreview(parent)
{
    setlocale(LC_NUMERIC, "C");

    playerWidget = new VideoWidget(this);
    statusBar = new VideoStatusBar(this);
}

VideoPreview::~VideoPreview()
{
    if (playerWidget->isVisible()) {
        playerWidget->hide();
        //        delete playerWidget.data();
        playerWidget->deleteLater();
    }

    if (statusBar) {
        statusBar->hide();
        statusBar->deleteLater();
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
    videoUrl = QUrl(url.toLocalFile());

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
    if (playerWidget->isVisible() && videoUrl.isValid()) {
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
