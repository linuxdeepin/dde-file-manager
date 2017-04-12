#include "imagepreviewplugin.h"

#include <QImageReader>
#include <QProcess>
#include <QMimeDatabase>
#include <QMimeType>
#include <QHBoxLayout>
#include <QLabel>

#include "imageview.h"
#include "messagetoobar.h"

#include "../../../utils/utils.h"

#define MIN_SIZE QSize(400, 300)

ImagePreviewPlugin::ImagePreviewPlugin(QObject *parent) : QObject(parent)
{
}

ImagePreviewPlugin::~ImagePreviewPlugin()
{
    qDebug () << "delete plugin";
}

void ImagePreviewPlugin::init(const QString &uri)
{
    m_uri = uri;
}

QWidget *ImagePreviewPlugin::previewWidget()
{
    return new ImageView(m_uri, 0);
}

QSize ImagePreviewPlugin::previewWidgetMinSize() const
{
    return QSize(600, 400);
}

bool ImagePreviewPlugin::canPreview() const
{
    QUrl url(m_uri);

    //check support mimetype
    QMimeDatabase mdatabase;
    QString mimetypeName = mdatabase.mimeTypeForFile(url.path()).name();
    if(QImageReader::supportedMimeTypes().contains(QByteArray(mimetypeName.toLocal8Bit().data()))){
        return true;
    }

    return false;
}

QWidget *ImagePreviewPlugin::toolBarItem()
{
    return new MessageTooBar(QUrl(m_uri).path());
}

QString ImagePreviewPlugin::pluginName() const
{
    return QString("dde-image-preview-plugin");
}

QIcon ImagePreviewPlugin::pluginLogo() const
{
    return QIcon();
}

QString ImagePreviewPlugin::pluginDescription() const
{
    return QString("Deepin image preview plugin");
}
