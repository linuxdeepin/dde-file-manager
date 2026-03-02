// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "iconutils.h"

#include <dfm-base/dfm_global_defines.h>

#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

DFMBASE_USE_NAMESPACE

QPixmap IconUtils::renderIconBackground(const QSize &size, const IconStyle &style)
{
    QPixmap pm { size };
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHints(p.renderHints() | QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);

    QRect rect { { 0, 0 }, size };
    p.drawRoundedRect(rect, style.radius, style.radius);
    p.end();
    return pm;
}

QPixmap IconUtils::addShadowToPixmap(const QPixmap &originalPixmap, int shadowOffsetY, qreal blurRadius, qreal shadowOpacity)
{
    auto size { originalPixmap.size() };
    size.rwidth() += (2 * blurRadius);
    size.rheight() += (2 * blurRadius);

    QGraphicsDropShadowEffect *shadowEffect { new QGraphicsDropShadowEffect };
    shadowEffect->setBlurRadius(blurRadius);   // 设置模糊半径
    shadowEffect->setColor(QColor(0, 0, 0, int(shadowOpacity * 255)));   // 设置颜色和透明度
    shadowEffect->setOffset(0, shadowOffsetY);   // 设置阴影偏移量

    QGraphicsScene scene;
    QGraphicsPixmapItem pixmapItem(originalPixmap);
    pixmapItem.setGraphicsEffect(shadowEffect);
    scene.addItem(&pixmapItem);

    QPixmap resultPixmap(size);
    resultPixmap.fill(Qt::transparent);
    QPainter painter(&resultPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    scene.render(&painter, QRectF(), QRectF(-blurRadius, -blurRadius, size.width(), size.height()));

    painter.end();

    delete shadowEffect;
    return resultPixmap;
}

IconUtils::IconStyle IconUtils::getIconStyle(int size)
{
    IconStyle style;
    const int normalizedSize = qMax(0, size);
    // Base ratio from 96px thumbnails:
    // stroke=3 (1/32), radius=6 (1/16), shadowOffset=2 (1/48), shadowRange=4 (1/24).
    style.stroke = qMax(1, normalizedSize / 32);
    style.radius = qMax(1, normalizedSize / 16);
    style.shadowOffset = qMax(1, normalizedSize / 48);
    style.shadowRange = qMax(1, normalizedSize / 24);

    return style;
}

QPixmap IconUtils::renderIconBackground(const QSizeF &size, const IconStyle &style)
{
    QSize pixSize(qRound(size.width()), qRound(size.height()));
    QPixmap pm { pixSize };
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHints(p.renderHints() | QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);

    QRectF rect { { 0, 0 }, size };
    p.drawRoundedRect(rect, style.radius, style.radius);
    p.end();
    return pm;
}

bool IconUtils::shouldSkipThumbnailFrame(const QString &mimeType)
{
    // appimage, uab 不显示缩略图底板
    static const QStringList kExcludedMimes = {
        Global::Mime::kTypeAppAppimage,
        Global::Mime::kTypeAppUab
    };
    return kExcludedMimes.contains(mimeType);
}
