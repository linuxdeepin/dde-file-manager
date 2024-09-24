// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "iconutils.h"

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

    if (size <= 48) {
        style.stroke = 1;
        style.radius = 2;
        style.shadowOffset = 1;
        style.shadowRange = 2;
    } else if (size <= 64) {
        style.stroke = 2;
        style.radius = 4;
        style.shadowOffset = 1;
        style.shadowRange = 3;
    } else if (size <= 96) {
        style.stroke = 3;
        style.radius = 6;
        style.shadowOffset = 2;
        style.shadowRange = 4;
    } else if (size <= 128) {
        style.stroke = 4;
        style.radius = 8;
        style.shadowOffset = 3;
        style.shadowRange = 5;
    } else if (size <= 160) {
        style.stroke = 5;
        style.radius = 10;
        style.shadowOffset = 3;
        style.shadowRange = 7;
    } else {
        style.stroke = 6;
        style.radius = 12;
        style.shadowOffset = 4;
        style.shadowRange = 8;
    }

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
