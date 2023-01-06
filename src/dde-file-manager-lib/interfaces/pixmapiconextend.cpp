// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "pixmapiconextend.h"
#include <private/qicon_p.h>
#include <QApplication>

PixmapIconExtend::PixmapIconExtend(const QIcon &other)
    : QIcon(other)
{
}

PixmapIconExtend::~PixmapIconExtend()
{
}

QPixmap PixmapIconExtend::pixmapExtend(const QSize &size, qreal devicePixelRatio, QIcon::Mode mode, QIcon::State state)
{
    DataPtr d = data_ptr();
    if (!d)
        return QPixmap();

    if (devicePixelRatio <= -1.0)
        devicePixelRatio = qApp->devicePixelRatio();

    if (devicePixelRatio > 1.0) {
        QPixmap pixmap = d->engine->pixmap(size, mode, state);
        pixmap.setDevicePixelRatio(1.0);
    }

    QPixmap pixmap = d->engine->scaledPixmap(size * devicePixelRatio, mode, state, devicePixelRatio);
    pixmap.setDevicePixelRatio(pixmapDevicePixelRatio(devicePixelRatio, size, pixmap.size()));
    return pixmap;
}

qreal PixmapIconExtend::pixmapDevicePixelRatio(qreal displayDevicePixelRatio, const QSize &requestedSize, const QSize &actualSize)
{
    QSize targetSize = requestedSize * displayDevicePixelRatio;
    qreal scale = 0.5 * (qreal(actualSize.width()) / qreal(targetSize.width()) + qreal(actualSize.height() / qreal(targetSize.height())));
    return qMax(qreal(1.0), displayDevicePixelRatio * scale);
}
