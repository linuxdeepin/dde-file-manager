// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "musicpreview.h"
#include "musicmessageview.h"
#include "toolbarframe.h"
#include "cusmediaplayer.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/mimetype/dmimedatabase.h>

#include <QFileInfo>
#include <QMimeType>
#include <QFrame>
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
using namespace plugin_filepreview;
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

void MusicPreview::handleBeforDestroy()
{
    emit CusMediaPlayer::instance()->sigStop();
}

bool MusicPreview::canPreview(const QUrl &url) const
{
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);

    if (!info)
        return false;
    QMimeType mimeType = DMimeDatabase().mimeTypeForUrl(url);

    return mimeType.name().startsWith("audio/");
}
