// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagepreviewwidget.h"

#include <QPainter>
#include <QApplication>

using namespace dfmplugin_detailspace;

ImagePreviewWidget::ImagePreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void ImagePreviewWidget::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    update();
}

QPixmap ImagePreviewWidget::pixmap() const
{
    return m_pixmap;
}

QSize ImagePreviewWidget::sizeHint() const
{
    int availableWidth = parentWidget() ? parentWidget()->width() : 280;
    int previewWidth = availableWidth - 2 * kPreviewMargin;
    int previewHeight = static_cast<int>(previewWidth / kPreviewAspectRatio);
    return QSize(previewWidth, previewHeight);
}

void ImagePreviewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QRect availableRect = rect().adjusted(kPreviewMargin, kPreviewMargin,
                                          -kPreviewMargin, -kPreviewMargin);

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
