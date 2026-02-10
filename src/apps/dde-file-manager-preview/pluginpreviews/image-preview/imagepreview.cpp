// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagepreview.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/utils/fileutils.h>
#include "imageview.h"

#include <DAnchors>

#include <QImageReader>
#include <QProcess>
#include <QMimeType>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileInfo>
#include <QMimeData>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace plugin_filepreview;

ImagePreview::ImagePreview(QObject *parent)
    : AbstractBasePreview(parent)
{
    fmInfo() << "Image preview: ImagePreview instance created";
}

ImagePreview::~ImagePreview()
{
    fmInfo() << "Image preview: ImagePreview instance destroyed";
    
    if (imageView)
        imageView->deleteLater();

    if (messageStatusBar)
        messageStatusBar->deleteLater();
}

bool ImagePreview::canPreview(const QUrl &url, QByteArray *format) const
{
    fmDebug() << "Image preview: checking if can preview:" << url;
    
    QByteArray f = QImageReader::imageFormat(url.toLocalFile());

    if (f.isEmpty()) {
        fmDebug() << "Image preview: QImageReader could not detect format, trying MIME database";
        DFMBASE_NAMESPACE::DMimeDatabase mimeDatabase;

        const QMimeType &mt = mimeDatabase.mimeTypeForFile(url, QMimeDatabase::MatchContent);

        f = mt.preferredSuffix().toLatin1();

        if (f.isEmpty()) {
            fmWarning() << "Image preview: no format detected for file:" << url;
            if (format) {
                *format = f;
            }
            return false;
        }
    }

    if (format) {
        *format = f;
    }

    bool supported = QImageReader::supportedImageFormats().contains(f);
    fmDebug() << "Image preview: format" << f << "supported:" << supported << "for file:" << url;
    return supported;
}

void ImagePreview::initialize(QWidget *window, QWidget *statusBar)
{
    Q_UNUSED(window)

    fmDebug() << "Image preview: initializing with status bar";
    
    messageStatusBar = new QLabel(statusBar);
    messageStatusBar->setStyleSheet("QLabel{font-family: Helvetica;\
                                   font-size: 12px;\
                                   font-weight: 300;}");

    DAnchorsBase(messageStatusBar).setCenterIn(statusBar);
    
    fmDebug() << "Image preview: status bar initialized";
}

bool ImagePreview::setFileUrl(const QUrl &url)
{
    fmInfo() << "Image preview: setting file URL:" << url;
    
    if (currentFileUrl == url) {
        fmDebug() << "Image preview: URL unchanged, skipping:" << url;
        return true;
    }

    QUrl tmpUrl = UrlRoute::fromLocalFile(url.path());
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    if (info.isNull()) {
        fmWarning() << "Image preview: failed to create FileInfo for:" << url;
        return false;
    }

    if (info->canAttributes(CanableInfoType::kCanRedirectionFileUrl)) {
        tmpUrl = info->urlOf(UrlInfoType::kRedirectedFileUrl);
        fmDebug() << "Image preview: using redirected URL:" << tmpUrl;
    }

    if (!tmpUrl.isLocalFile()) {
        fmWarning() << "Image preview: URL is not a local file:" << tmpUrl;
        return false;
    }

    QByteArray format;
    if (!canPreview(tmpUrl, &format)) {
        fmWarning() << "Image preview: cannot preview file:" << tmpUrl << "format:" << format;
        return false;
    }

    currentFileUrl = tmpUrl;

    if (!imageView) {
        fmDebug() << "Image preview: creating new ImageView for:" << tmpUrl.toLocalFile() << "format:" << format;
        imageView = new ImageView(tmpUrl.toLocalFile(), format);
    } else {
        fmDebug() << "Image preview: updating existing ImageView with:" << tmpUrl.toLocalFile() << "format:" << format;
        imageView->setFile(tmpUrl.toLocalFile(), format);
    }

    const QSize &image_size = imageView->sourceSize();
    fmDebug() << "Image preview: image size:" << image_size;

    messageStatusBar->setText(QString("%1x%2").arg(image_size.width()).arg(image_size.height()));
    messageStatusBar->adjustSize();

    imageTitle = QFileInfo(tmpUrl.toLocalFile()).fileName();

    fmInfo() << "Image preview: file URL set successfully:" << url << "title:" << imageTitle;
    Q_EMIT titleChanged();

    return true;
}

QUrl ImagePreview::fileUrl() const
{
    return currentFileUrl;
}

QWidget *ImagePreview::contentWidget() const
{
    return imageView;
}

QString ImagePreview::title() const
{
    return imageTitle;
}
