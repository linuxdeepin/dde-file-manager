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

DFM_BEGIN_NAMESPACE

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

bool MusicPreview::canPreview(const DUrl &url) const
{
    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, url);

    if (!info)
        return false;

    return QMediaPlayer::hasSupport(info->mimeTypeName(QMimeDatabase::MatchContent)) != QMultimedia::NotSupported;
}

DFM_END_NAMESPACE
