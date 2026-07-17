// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
    const qreal targetDpr = qMax<qreal>(1.0, qApp->devicePixelRatio());

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
        setMinimumSize(MIN_SIZE);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    }

    if (movie) {
        QMovie *tmpMovie = movie;
        movie = nullptr;
        tmpMovie->stop();
        tmpMovie->disconnect();
        tmpMovie->deleteLater();
    }

    // 使用 QImageReader 进行高效加载，避免大图片内存溢出
    QImageReader reader(fileName, format);
    reader.setAutoTransform(true);

    QImage image = reader.read();
    if (image.isNull()) {
        fmWarning() << "Image preview: failed to load image:" << reader.errorString();
        setPixmap(QPixmap());
        return;
    }
    // tga/icns 等插件未实现尺寸探测接口，QImageReader.size() 返回 (-1,-1)，需实际读取以获取真实尺寸
    sourceImageSize = image.size();
    if (!sourceImageSize.isValid()) {
        fmWarning() << "Image preview: the image size is not valid";
        setPixmap(QPixmap());
        return;
    }

    // 计算保持宽高比的显示尺寸
    QSize maxShowSize(qMin(static_cast<int>(dsize.width() * 0.7), sourceImageSize.width()),
                      qMin(static_cast<int>(dsize.height() * 0.7), sourceImageSize.height()));
    QSize showSize = sourceImageSize.scaled(maxShowSize, Qt::KeepAspectRatio);

    // Keep the widget size in logical pixels, but decode extra backing pixels for HiDPI.
    QSize decodeSize(qMax(1, qRound(showSize.width() * targetDpr)),
                     qMax(1, qRound(showSize.height() * targetDpr)));
    // reader 已完成读取，setScaledSize 不再生效，直接将已加载的 image 缩放到 decodeSize，
    // 保留 HiDPI 所需的额外像素，同时避免持有全分辨率图像造成内存浪费。
    image = image.scaled(decodeSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // 应用颜色空间转换，解决CMYK等格式的颜色显示问题 (仅Qt6)
    image = DFMBASE_NAMESPACE::FileUtils::convertToSRgbColorSpace(image);
#endif
    QPixmap pixmap = QPixmap::fromImage(image);
    const qreal widthDpr = showSize.width() > 0 ? static_cast<qreal>(image.width()) / showSize.width() : 1.0;
    const qreal heightDpr = showSize.height() > 0 ? static_cast<qreal>(image.height()) / showSize.height() : 1.0;
    const qreal effectiveDpr = qMax<qreal>(1.0, qMin(targetDpr, qMin(widthDpr, heightDpr)));
    pixmap.setDevicePixelRatio(effectiveDpr);
    setPixmap(pixmap);
}

QSize ImageView::sourceSize() const
{
    return sourceImageSize;
}
