// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "iconutils.h"

#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

DFMBASE_USE_NAMESPACE

QPixmap IconUtils::renderIconBackground(const QSize &size)
{
    QPixmap pm { size };
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHints(p.renderHints() | QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);

    QRect rect { { 0, 0 }, size };
    p.drawRoundedRect(rect, 6, 6);
    p.end();
    return addShadowToPixmap(pm, 1, 2, 0.2);
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
