// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
    fmInfo() << "Music preview: MusicPreview instance created";
}

MusicPreview::~MusicPreview()
{
    fmInfo() << "Music preview: MusicPreview instance destroyed";
    
    if (musicView)
        musicView->deleteLater();

    if (statusBarFrame)
        statusBarFrame->deleteLater();
}

bool MusicPreview::setFileUrl(const QUrl &url)
{
    fmInfo() << "Music preview: setting file URL:" << url;
    
    if (currentUrl == url) {
        fmDebug() << "Music preview: URL unchanged, skipping:" << url;
        return true;
    }

    if (!url.isLocalFile()) {
        fmWarning() << "Music preview: URL is not a local file:" << url;
        return false;
    }

    if (musicView || statusBarFrame) {
        fmWarning() << "Music preview: widgets already exist, cannot set new URL:" << url;
        return false;
    }

    if (!canPreview(url)) {
        fmWarning() << "Music preview: cannot preview file:" << url;
        return false;
    }

    currentUrl = url;

    fmDebug() << "Music preview: creating music view and toolbar for:" << url;
    musicView = new MusicMessageView(url.toString());
    statusBarFrame = new ToolBarFrame(url.toString());
    musicView->setFixedSize(600, 336);
    statusBarFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    fmInfo() << "Music preview: file URL set successfully:" << url;
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
    fmDebug() << "Music preview: starting playback";
    statusBarFrame->play();
}

void MusicPreview::pause()
{
    fmDebug() << "Music preview: pausing playback";
    statusBarFrame->pause();
}

void MusicPreview::stop()
{
    fmDebug() << "Music preview: stopping playback";
    statusBarFrame->stop();
}

void MusicPreview::handleBeforDestroy()
{
    fmDebug() << "Music preview: handling before destroy, stopping playback";
    emit CusMediaPlayer::instance()->sigStop();
}

bool MusicPreview::canPreview(const QUrl &url) const
{
    fmDebug() << "Music preview: checking if can preview:" << url;
    
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);

    if (!info) {
        fmWarning() << "Music preview: failed to create FileInfo for:" << url;
        return false;
    }
    
    QMimeType mimeType = DMimeDatabase().mimeTypeForUrl(url);
    bool canPreview = mimeType.name().startsWith("audio/");
    
    fmDebug() << "Music preview: mime type:" << mimeType.name() << "can preview:" << canPreview;
    return canPreview;
}
