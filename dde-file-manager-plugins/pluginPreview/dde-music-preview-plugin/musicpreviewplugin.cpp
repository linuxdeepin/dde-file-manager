#include "musicpreviewplugin.h"

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

MusicPreviewPlugin::MusicPreviewPlugin(QObject *parent) :
    PreviewInterface(parent)
{
    m_supportSuffixed << "mp3"
                      << "mpc"
                      << "flac"
                      << "trueaudio"
                      << "wavpack"
                      << "ogg"
                      << "speex";
}

void MusicPreviewPlugin::init(const QString &uri)
{
    m_uri = uri;
}

QWidget *MusicPreviewPlugin::previewWidget()
{
    return new MusicMessageView(m_uri);
}

QSize MusicPreviewPlugin::previewWidgetMinSize() const
{
    return QSize(600, 336);
}

bool MusicPreviewPlugin::canPreview() const
{
    QFileInfo info(QUrl(m_uri).path());
    if(m_supportSuffixed.contains(info.suffix())){
        return true;
    }

    QMimeDatabase md;
    QString mimeType = md.mimeTypeForFile(QUrl(m_uri).path()).name();
    if(mimeType.startsWith("audio")){
        return true;
    }

    return false;
}

QWidget *MusicPreviewPlugin::toolBarItem()
{
    return new ToolBarFrame(m_uri);
}

QString MusicPreviewPlugin::pluginName() const
{
    return "dde-music-preview-plugin";
}

QIcon MusicPreviewPlugin::pluginLogo() const
{
    return QIcon();
}

QString MusicPreviewPlugin::pluginDescription() const
{
    return "Deepin music preview plugin";
}
