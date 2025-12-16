// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageview.h"

#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/fileutils.h>

#include <QUrl>
#include <QImageReader>
#include <QApplication>
#include <QtMath>
#include <QPainter>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QMovie>
#include <QScreen>

using namespace plugin_filepreview;
#define MIN_SIZE QSize(400, 300)

ImageView::ImageView(const QString &fileName, const QByteArray &format, QWidget *parent)
    : QLabel(parent)
{
    setFile(fileName, format);
    setMinimumSize(MIN_SIZE);
    setAlignment(Qt::AlignCenter);
}

void ImageView::setFile(const QString &fileName, const QByteArray &format)
{
    const QSize &dsize = DFMBASE_NAMESPACE::WindowUtils::cursorScreen()->size();

    if (format == QByteArrayLiteral("gif")) {
        if (movie) {
            movie->stop();   // blumia: we need to stop it first before we load a new file
            movie->setFileName(fileName);
        } else {
            movie = new QMovie(fileName, format, this);
        }
        setMovie(movie);
        movie->start();
        sourceImageSize = movie->frameRect().size();
        QSize showSize = QSize(qMin(static_cast<int>(dsize.width() * 0.7), sourceImageSize.width()),
                               qMin(static_cast<int>(dsize.height() * 0.7), sourceImageSize.height()));
        setFixedSize(showSize);
        movie->setScaledSize(showSize);
        return;
    } else {
        setMovie(nullptr);
    }

    if (movie) {
        QMovie *tmpMovie = movie;
        movie = nullptr;
        tmpMovie->stop();
        tmpMovie->disconnect();
        tmpMovie->deleteLater();
    }

    QImage image(fileName, format);
    sourceImageSize = image.size();
    if (!sourceImageSize.isValid()) {
        setPixmap(QPixmap());
        return;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // 应用颜色空间转换，解决CMYK等格式的颜色显示问题 (仅Qt6)
    image = DFMBASE_NAMESPACE::FileUtils::convertToSRgbColorSpace(image);
#endif
    QSize showSize = QSize(qMin(static_cast<int>(dsize.width() * 0.7), sourceImageSize.width()),
                           qMin(static_cast<int>(dsize.height() * 0.7), sourceImageSize.height()));
    QPixmap pixmap = QPixmap::fromImage(image).scaled(showSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
    setPixmap(pixmap);
}

QSize ImageView::sourceSize() const
{
    return sourceImageSize;
}
