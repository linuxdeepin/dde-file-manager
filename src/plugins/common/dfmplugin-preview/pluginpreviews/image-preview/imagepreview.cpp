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

#include "imagepreview.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/mimetype/dmimedatabase.h"
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

        const QMimeType &mt = mimeDatabase.mimeTypeForFile(url.toLocalFile(), QMimeDatabase::MatchContent);

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
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    if (info.isNull())
        return false;

    if (info->canRedirectionFileUrl()) {
        tmpUrl = info->redirectedFileUrl();
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
