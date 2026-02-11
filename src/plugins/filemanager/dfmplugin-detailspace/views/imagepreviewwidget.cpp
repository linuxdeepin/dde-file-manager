// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagepreviewwidget.h"
#include "detailspacewidget.h"

#include <QImageReader>
#include <QMovie>
#include <QPainter>
#include <QApplication>

using namespace dfmplugin_detailspace;

ImagePreviewWidget::ImagePreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

ImagePreviewWidget::~ImagePreviewWidget()
{
    stopAnimatedImage();
}

void ImagePreviewWidget::setPixmap(const QPixmap &pixmap)
{
    // Stop and release any animated image resources before switching to static pixmap
    stopAnimatedImage();

    m_pixmap = pixmap;
    update();
}

QPixmap ImagePreviewWidget::pixmap() const
{
    return m_pixmap;
}

void ImagePreviewWidget::setAnimatedImage(const QString &filePath)
{
    if (!m_movie) {
        m_movie = new QMovie(this);
        connect(m_movie, &QMovie::frameChanged, this, &ImagePreviewWidget::onMovieFrameChanged);
    }

    if (m_movie->fileName() != filePath) {
        m_movie->stop();
        m_movie->setFileName(filePath);
    }

    m_hasAnimatedImage = m_movie->isValid() && (m_movie->frameCount() > 1);

    // Start animation if valid
    if (m_hasAnimatedImage && m_movie->state() != QMovie::Running) {
        m_movie->start();
    }
}

void ImagePreviewWidget::stopAnimatedImage()
{
    if (m_hasAnimatedImage && m_movie) {
        m_movie->stop();
        m_movie->setFileName("");  // Release file descriptor to allow device unmounting
        m_hasAnimatedImage = false;
    }
}

QSize ImagePreviewWidget::sizeHint() const
{
    // Parent widget is expandFrame inside scrollArea
    int availableWidth = parentWidget() ? parentWidget()->width() : 260;

    // Apply internal preview margins
    int previewWidth = availableWidth;
    int previewHeight = static_cast<int>(previewWidth / kPreviewAspectRatio);
    return QSize(previewWidth, previewHeight);
}

bool ImagePreviewWidget::isAnimatedMimeType(const QString &mimeType)
{
    const QList<QByteArray> imageFormats = QImageReader::imageFormatsForMimeType(mimeType.toUtf8());
    return std::any_of(imageFormats.begin(), imageFormats.end(),
                       [](const QByteArray &format) { return format == QByteArrayLiteral("gif"); });
}

QSize ImagePreviewWidget::maximumPreviewSize()
{
    // Calculate preview size for maximum DetailSpace width (500px)
    // This is the single source of truth for preview size calculation
    int maxPreviewWidth = DetailSpaceWidget::kMaximumWidth - 2 * kPreviewMargin;
    int maxPreviewHeight = static_cast<int>(maxPreviewWidth / kPreviewAspectRatio);
    return QSize(maxPreviewWidth, maxPreviewHeight);
}

void ImagePreviewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    const QRect availableRect = rect();

    if (m_pixmap.isNull()) {
        return;
    }

    qreal dpr = m_pixmap.devicePixelRatio();
    QSize pixmapLogicalSize = m_pixmap.size() / dpr;
    QSize scaledSize = pixmapLogicalSize.scaled(availableRect.size(), Qt::KeepAspectRatio);

    // Do not scale up beyond original size
    if (scaledSize.width() > pixmapLogicalSize.width()) {
        scaledSize = pixmapLogicalSize;
    }

    QRect targetRect(QPoint(0, 0), scaledSize);
    targetRect.moveCenter(availableRect.center());

    painter.drawPixmap(targetRect, m_pixmap);
}

void ImagePreviewWidget::onMovieFrameChanged()
{
    if (m_movie) {
        m_pixmap = m_movie->currentPixmap();
        update();
    }
}
