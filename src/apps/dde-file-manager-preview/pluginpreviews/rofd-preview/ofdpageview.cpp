// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ofdpageview.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPalette>
#include <QResizeEvent>

namespace plugin_filepreview {

OfdPageView::OfdPageView(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);
}

QSize OfdPageView::sizeHint() const
{
    // The preview dialog sizes itself to this widget; provide a sensible
    // default window size before the first page finishes rendering.
    return QSize(860, 640);
}

void OfdPageView::setPage(const QImage &image)
{
    busy = false;
    pagePixmap = QPixmap::fromImage(image);
    update();
}

void OfdPageView::setBusy(bool busy)
{
    this->busy = busy;
    update();
}

void OfdPageView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (pagePixmap.isNull()) {
        return;
    }

    if (busy) {
        painter.fillRect(rect(), QColor(255, 255, 255, 150));
    }

    const QSize target = pagePixmap.size().scaled(size(), Qt::KeepAspectRatio);
    const QRect drawRect(QPoint((width() - target.width()) / 2, (height() - target.height()) / 2), target);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(drawRect, pagePixmap);
}

void OfdPageView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    Q_EMIT viewportResized();
}
}
