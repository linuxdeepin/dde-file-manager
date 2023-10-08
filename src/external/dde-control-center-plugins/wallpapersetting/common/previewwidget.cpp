// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewwidget.h"
#include "commondefine.h"

#include <QPainter>
#include <QBitmap>
#include <QTime>
#include <QDebug>

using namespace dfm_wallpapersetting;

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    bkgColor = QColor("#868686");
}

void PreviewWidget::updateSize()
{
    // 高耗时
    pixmap = QPixmap();
    QPixmap pix(imgPath);
    if (pix.isNull())
        return;

    pixmap = scaledPixmap(pix);
}

void PreviewWidget::setImage(const QString &img)
{
    imgPath = img;
    updateSize();
}

void PreviewWidget::setBackground(const QColor &color)
{
    bkgColor = color;
}

void PreviewWidget::setBoder(const QColor &color)
{
    bdColor = color;
}

QPixmap PreviewWidget::scaledPixmap(const QPixmap &pixmap) const
{
    const int border = PREVIEW_ICON_MARGIN * 2;
    QSize orgSize = (size() - QSize(border, border)) * devicePixelRatioF();
    auto pix = pixmap.scaled(orgSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pix.setDevicePixelRatio(devicePixelRatioF());
    return pix;
}

void PreviewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateSize();
}

void PreviewWidget::paintEvent(QPaintEvent *event)
{
    const int margin = PREVIEW_ICON_MARGIN;
    QPainter pa(this);
    pa.setRenderHint(QPainter::Antialiasing);

    // draw border
    QRect bkgRect(QPoint(0, 0), size());
    {
        pa.setBrush(bdColor);
        pa.setPen(Qt::NoPen);
        pa.drawRoundedRect(bkgRect, PREVIEW_BORDER_ROUND, PREVIEW_BORDER_ROUND);
    }

    bkgRect = bkgRect.marginsRemoved(QMargins(margin, margin, margin, margin));
    if (pixmap.isNull()) {
        // draw background
        pa.setBrush(bkgColor);
        pa.drawRoundedRect(bkgRect, PREVIEW_ICON_ROUND, PREVIEW_ICON_ROUND);
    } else {
        QPainterPath roundedPath;
        roundedPath.addRoundedRect(bkgRect, PREVIEW_ICON_ROUND, PREVIEW_ICON_ROUND);
        pa.setClipPath(roundedPath);
        pa.drawPixmap(bkgRect, pixmap);
    }
}

