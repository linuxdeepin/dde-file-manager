// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemdelegatehelper.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/iconutils.h>

#include <QPainter>
#include <QApplication>
#include <QPainterPath>

#include <cmath>

#define CEIL(x) (static_cast<int>(std::ceil(x)))

using namespace dfmplugin_workspace;
using namespace dfmbase;

/*!
 * \brief getIconPixmap 获取icon的pixmap
 *
 * \return QPixmap icon的pixmap图片
 **/
QPixmap ItemDelegateHelper::getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio = 1.0, QIcon::Mode mode, QIcon::State state)
{
    if (icon.isNull())
        return QPixmap();

    // 确保当前参数参入获取图片大小大于0
    if (size.width() <= 0 || size.height() <= 0)
        return QPixmap();

    auto px = icon.pixmap(size, mode, state);
    px.setDevicePixelRatio(qApp->devicePixelRatio());

    return px;
}
/*!
 * \brief paintIcon 绘制指定区域内每一个icon的pixmap
 *
 * \return void
 **/
void ItemDelegateHelper::paintIcon(QPainter *painter, const QIcon &icon, const PaintIconOpts &opts)
{
    // Copy of QStyle::alignedRect
    Qt::Alignment alignment = visualAlignment(painter->layoutDirection(), opts.alignment);
    const qreal pixelRatio = painter->device()->devicePixelRatioF();
    const QPixmap &px = getIconPixmap(icon, opts.rect.size().toSize(), pixelRatio, opts.mode, opts.state);
    qreal x = opts.rect.x();
    qreal y = opts.rect.y();
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += (opts.rect.size().height() - h) / 2.0;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += opts.rect.size().height() - h;
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += opts.rect.size().width() - w;
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += (opts.rect.size().width() - w) / 2.0;

    // Task: 337513
    if (opts.viewMode == ViewMode::kIconMode && opts.isThumb) {
        painter->save();
        painter->setRenderHints(painter->renderHints() | QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);

        QRect backgroundRect { qRound(x), qRound(y), qRound(w), qRound(h) };
        QRect imageRect { backgroundRect };
        // 绘制带有阴影的背景
        painter->drawPixmap(backgroundRect, IconUtils::renderIconBackground(backgroundRect.size()));

        // 绘制缩略图(上下左右各缩小2px)
        imageRect.adjust(4, 4, -4, -4);
        QPainterPath clipPath;
        clipPath.addRoundedRect(imageRect, 4, 4);
        painter->setClipPath(clipPath);
        painter->drawPixmap(imageRect, px);
        painter->restore();

    } else {
        painter->drawPixmap(qRound(x), qRound(y), px);
    }
}

void ItemDelegateHelper::drawBackground(const qreal &backgroundRadius, const QRectF &rect, QRectF &lastLineRect, const QBrush &backgroundBrush, QPainter *painter)
{
    const QMarginsF margins(backgroundRadius, 0, backgroundRadius, 0);
    QRectF backBounding = rect;
    QPainterPath path;
    if (lastLineRect.isValid()) {
        if (qAbs(rect.width() - lastLineRect.width()) < backgroundRadius * 2) {
            backBounding.setWidth(lastLineRect.width());
            backBounding.moveCenter(rect.center());
            path.moveTo(lastLineRect.x() - backgroundRadius, lastLineRect.bottom() - backgroundRadius);
            path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius);
            path.lineTo(lastLineRect.right() + backgroundRadius, backBounding.bottom() - backgroundRadius);
            path.arcTo(backBounding.right() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 0, -90);
            path.lineTo(backBounding.x(), backBounding.bottom());
            path.arcTo(backBounding.x() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, -90);
            lastLineRect = backBounding;
        } else if (lastLineRect.width() > rect.width()) {
            backBounding += margins;
            path.moveTo(backBounding.x() - backgroundRadius, backBounding.y() - 1);
            path.arcTo(backBounding.x() - backgroundRadius * 2, backBounding.y() - 1, backgroundRadius * 2, backgroundRadius * 2 + 1, 90, -90);
            path.lineTo(backBounding.x(), backBounding.bottom() - backgroundRadius);
            path.arcTo(backBounding.x(), backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 180, 90);
            path.lineTo(backBounding.right() - backgroundRadius, backBounding.bottom());
            path.arcTo(backBounding.right() - backgroundRadius * 2, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, 90);
            path.lineTo(backBounding.right(), backBounding.top() + backgroundRadius);
            path.arcTo(backBounding.right(), backBounding.top() - 1, backgroundRadius * 2, backgroundRadius * 2 + 1, 180, -90);
            path.closeSubpath();
            lastLineRect = rect;
        } else {
            backBounding += margins;
            path.moveTo(lastLineRect.x() - backgroundRadius * 2, lastLineRect.bottom());
            path.arcTo(lastLineRect.x() - backgroundRadius * 3, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, 90);
            path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2);
            path.arcTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 180, 90);
            path.addRoundedRect(backBounding, backgroundRadius, backgroundRadius);
            lastLineRect = rect;
        }
    } else {
        lastLineRect = backBounding;
        path.addRoundedRect(backBounding + margins, backgroundRadius, backgroundRadius);
    }

    bool isAntialiasing = painter->testRenderHint(QPainter::Antialiasing);
    qreal painterOpacity = painter->opacity();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setOpacity(1);
    painter->fillPath(path, backgroundBrush);
    painter->setRenderHint(QPainter::Antialiasing, isAntialiasing);
    painter->setOpacity(painterOpacity);
}

void ItemDelegateHelper::hideTooltipImmediately()
{
    QWidgetList qwl = QApplication::topLevelWidgets();
    for (QWidget *qw : qwl) {
        if (QStringLiteral("QTipLabel") == qw->metaObject()->className()) {
            qw->close();
        }
    }
}

ElideTextLayout *ItemDelegateHelper::createTextLayout(const QString &name, QTextOption::WrapMode wordWrap,
                                                      qreal lineHeight, int alignmentFlag, QPainter *painter)
{
    ElideTextLayout *layout = new ElideTextLayout(name);

    layout->setAttribute(ElideTextLayout::kWrapMode, wordWrap);
    layout->setAttribute(ElideTextLayout::kLineHeight, lineHeight);
    layout->setAttribute(ElideTextLayout::kAlignment, alignmentFlag);

    if (painter) {
        layout->setAttribute(ElideTextLayout::kFont, painter->font());
        layout->setAttribute(ElideTextLayout::kTextDirection, painter->layoutDirection());
    }

    return layout;
}
