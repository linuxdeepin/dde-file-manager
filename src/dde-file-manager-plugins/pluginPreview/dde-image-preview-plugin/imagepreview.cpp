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

#include "imagepreview.h"
#include "dabstractfileinfo.h"
#include "dfileservices.h"

#include <QImageReader>
#include <QProcess>
#include <QMimeDatabase>
#include <QMimeType>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileInfo>
#include <QMimeData>

#include <danchors.h>

#include "imageview.h"

DWIDGET_USE_NAMESPACE

DFM_USE_NAMESPACE

ImagePreview::ImagePreview(QObject *parent)
    : DFMFilePreview(parent)
{
}

ImagePreview::~ImagePreview()
{
    if (m_imageView)
        m_imageView->deleteLater();

    if (m_messageStatusBar)
        m_messageStatusBar->deleteLater();
}

bool ImagePreview::canPreview(const QUrl &url, QByteArray *format) const
{
    QByteArray f = QImageReader::imageFormat(url.toLocalFile());

    if (f.isEmpty()) {
        QMimeDatabase mimeDatabase;

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

    m_messageStatusBar = new QLabel(statusBar);
    m_messageStatusBar->setStyleSheet("QLabel{font-family: Helvetica;\
                                   font-size: 12px;\
                                   font-weight: 300;}");

    DAnchorsBase(m_messageStatusBar).setCenterIn(statusBar);
}

bool ImagePreview::setFileUrl(const DUrl &url)
{
    if (m_url == url)
        return true;

    DUrl tmpUrl = url;
    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, url);

    if (!info)
        return false;

    if (info->canRedirectionFileUrl()) {
        tmpUrl = info->redirectedFileUrl();
    }

    if (!tmpUrl.isLocalFile())
        return false;

    QByteArray format;

    if (!canPreview(tmpUrl, &format))
        return false;

    m_url = tmpUrl;

    if (!m_imageView)
        m_imageView = new ImageView(tmpUrl.toLocalFile(), format);
    else
        m_imageView->setFile(tmpUrl.toLocalFile(), format);

    const QSize &image_size = m_imageView->sourceSize();

    m_messageStatusBar->setText(QString("%1x%2").arg(image_size.width()).arg(image_size.height()));
    m_messageStatusBar->adjustSize();

    m_title = QFileInfo(tmpUrl.toLocalFile()).fileName();

    Q_EMIT titleChanged();

    return true;
}

DUrl ImagePreview::fileUrl() const
{
    return m_url;
}

QWidget *ImagePreview::contentWidget() const
{
    return m_imageView;
}

QString ImagePreview::title() const
{
    return m_title;
}

void ImagePreview::copyFile() const
{
    QMimeData *data = new QMimeData();
    QImage image(m_url.toLocalFile());

    if (!image.isNull())
        data->setImageData(image);

    DFMGlobal::setUrlsToClipboard({m_url}, DFMGlobal::CopyAction, data);
}
