// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
}

ImagePreview::~ImagePreview()
{
    if (imageView)
        imageView->deleteLater();

    if (messageStatusBar)
        messageStatusBar->deleteLater();
}

bool ImagePreview::canPreview(const QUrl &url, QByteArray *format) const
{
    QByteArray f = QImageReader::imageFormat(url.toLocalFile());

    if (f.isEmpty()) {
        DFMBASE_NAMESPACE::DMimeDatabase mimeDatabase;

        const QMimeType &mt = mimeDatabase.mimeTypeForFile(url, QMimeDatabase::MatchContent);

        f = mt.preferredSuffix().toLatin1();

        if (f.isEmpty()) {
            if (format) {
                *format = f;
            }

            return false;
        }
    }

    if (format) {
        *format = f;
    }

    return QImageReader::supportedImageFormats().contains(f);
}

void ImagePreview::initialize(QWidget *window, QWidget *statusBar)
{
    Q_UNUSED(window)

    messageStatusBar = new QLabel(statusBar);
    messageStatusBar->setStyleSheet("QLabel{font-family: Helvetica;\
                                   font-size: 12px;\
                                   font-weight: 300;}");

    DAnchorsBase(messageStatusBar).setCenterIn(statusBar);
}

bool ImagePreview::setFileUrl(const QUrl &url)
{
    if (currentFileUrl == url)
        return true;

    QUrl tmpUrl = UrlRoute::fromLocalFile(url.path());
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    if (info.isNull())
        return false;

    if (info->canAttributes(CanableInfoType::kCanRedirectionFileUrl)) {
        tmpUrl = info->urlOf(UrlInfoType::kRedirectedFileUrl);
    }

    if (!tmpUrl.isLocalFile())
        return false;

    QByteArray format;

    if (!canPreview(tmpUrl, &format))
        return false;

    currentFileUrl = tmpUrl;

    if (!imageView)
        imageView = new ImageView(tmpUrl.toLocalFile(), format);
    else
        imageView->setFile(tmpUrl.toLocalFile(), format);

    const QSize &image_size = imageView->sourceSize();

    messageStatusBar->setText(QString("%1x%2").arg(image_size.width()).arg(image_size.height()));
    messageStatusBar->adjustSize();

    imageTitle = QFileInfo(tmpUrl.toLocalFile()).fileName();

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
