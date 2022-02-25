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

#include "musicpreview.h"
#include "musicmessageview.h"
#include "toolbarframe.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/base/schemefactory.h"

#include <QFileInfo>
#include <QMimeType>
#include <QMimeDatabase>
#include <QFrame>
#include <QSlider>
#include <QLabel>
#include <QUrl>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QThread>
#include <QImage>
#include <QApplication>

#include <taglib/taglib.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <taglib/fileref.h>
#include <taglib/id3v2header.h>
#include <taglib/attachedpictureframe.h>

#include <taglib/mp4file.h>
#include <taglib/mp4item.h>
#include <taglib/mp4coverart.h>
#include <taglib/mp4properties.h>

DFMBASE_USE_NAMESPACE
PREVIEW_USE_NAMESPACE
MusicPreview::MusicPreview(QObject *parent)
    : AbstractBasePreview(parent)
{
}

MusicPreview::~MusicPreview()
{
    if (musicView)
        musicView->deleteLater();

    if (statusBarFrame)
        statusBarFrame->deleteLater();
}

bool MusicPreview::setFileUrl(const QUrl &url)
{
    if (currentUrl == url)
        return true;

    if (!url.isLocalFile())
        return false;

    if (musicView || statusBarFrame)
        return false;

    if (!canPreview(url))
        return false;

    currentUrl = url;

    musicView = new MusicMessageView(url.toString());
    statusBarFrame = new ToolBarFrame(url.toString());

    musicView->setFixedSize(600, 336);
    statusBarFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    return true;
}

QUrl MusicPreview::fileUrl() const
{
    return currentUrl;
}

QWidget *MusicPreview::contentWidget() const
{
    return musicView;
}

QWidget *MusicPreview::statusBarWidget() const
{
    return statusBarFrame;
}

Qt::Alignment MusicPreview::statusBarWidgetAlignment() const
{
    return Qt::Alignment();
}

void MusicPreview::play()
{
    statusBarFrame->play();
}

void MusicPreview::pause()
{
    statusBarFrame->pause();
}

void MusicPreview::stop()
{
    statusBarFrame->stop();
}

bool MusicPreview::canPreview(const QUrl &url) const
{
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);

    if (!info)
        return false;
    QMimeType mimeType = MimeDatabase::mimeTypeForUrl(url);

    return QMediaPlayer::hasSupport(mimeType.name()) != QMultimedia::NotSupported;
}
