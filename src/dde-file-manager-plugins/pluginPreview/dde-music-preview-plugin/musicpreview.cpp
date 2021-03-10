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

#include "musicpreview.h"

#include "dfileservices.h"
#include "dabstractfileinfo.h"

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

#include "toolbarframe.h"
#include "musicmessageview.h"

#include "taglib/taglib.h"
#include "taglib/tag.h"
#include "taglib/tpropertymap.h"
#include "taglib/fileref.h"
#include "taglib/id3v2header.h"
#include "taglib/attachedpictureframe.h"

#include "taglib/mp4file.h"
#include "taglib/mp4item.h"
#include "taglib/mp4coverart.h"
#include "taglib/mp4properties.h"

MusicPreview::MusicPreview(QObject *parent)
    : DFMFilePreview(parent)
{

}

MusicPreview::~MusicPreview()
{
    if (m_musicView)
        m_musicView->deleteLater();

    if (m_statusBarFrame)
        m_statusBarFrame->deleteLater();
}

bool MusicPreview::setFileUrl(const DUrl &url)
{
    if (m_url == url)
        return true;

    if (!url.isLocalFile())
        return false;

    if (m_musicView || m_statusBarFrame)
        return false;

    if (!canPreview(url))
        return false;

    m_url = url;

    m_musicView = new MusicMessageView(url.toString());
    m_statusBarFrame = new ToolBarFrame(url.toString());

    m_musicView->setFixedSize(600, 336);
    m_statusBarFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    return true;
}

DUrl MusicPreview::fileUrl() const
{
    return m_url;
}

QWidget *MusicPreview::contentWidget() const
{
    return m_musicView;
}

QWidget *MusicPreview::statusBarWidget() const
{
    return m_statusBarFrame;
}

Qt::Alignment MusicPreview::statusBarWidgetAlignment() const
{
    return Qt::Alignment();
}

void MusicPreview::play()
{
    m_statusBarFrame->play();
}

void MusicPreview::pause()
{
    m_statusBarFrame->pause();
}

void MusicPreview::stop()
{
    m_statusBarFrame->stop();
}

bool MusicPreview::canPreview(const DUrl &url) const
{
    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, url);

    if (!info)
        return false;

    return QMediaPlayer::hasSupport(info->mimeTypeName(QMimeDatabase::MatchContent)) != QMultimedia::NotSupported;
}
