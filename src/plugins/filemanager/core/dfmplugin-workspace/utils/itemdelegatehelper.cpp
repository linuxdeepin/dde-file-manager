/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "itemdelegatehelper.h"

#include <dfm-base/utils/fileutils.h>

#include <QPainter>
#include <QApplication>
#include <QTextCodec>
#include <QPainterPath>
#include <private/qtextengine_p.h>

#include <cmath>

#define CEIL(x) (static_cast<int>(std::ceil(x)))

using namespace dfmplugin_workspace;

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
void ItemDelegateHelper::paintIcon(QPainter *painter, const QIcon &icon, const QRectF &rect, Qt::Alignment alignment, QIcon::Mode mode, QIcon::State state)
{
    // Copy of QStyle::alignedRect
    alignment = visualAlignment(painter->layoutDirection(), alignment);
    const qreal pixelRatio = painter->device()->devicePixelRatioF();
    const QPixmap &px = getIconPixmap(icon, rect.size().toSize(), pixelRatio, mode, state);
    qreal x = rect.x();
    qreal y = rect.y();
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += (rect.size().height() - h) / 2.0;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += rect.size().height() - h;
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += rect.size().width() - w;
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += (rect.size().width() - w) / 2.0;

    painter->drawPixmap(qRound(x), qRound(y), px);
}

QString ItemDelegateHelper::elideText(const QString &text, const QSizeF &size, QTextOption::WrapMode wordWrap, const QFont &font, Qt::TextElideMode mode, qreal lineHeight, qreal flags)
{
    QStringList lines;
    QTextLayout textLayout(text);

    textLayout.setFont(font);
    elideText(&textLayout, size, wordWrap, mode, lineHeight, static_cast<int>(flags), &lines);

    return lines.join('\n');
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

void ItemDelegateHelper::elideText(QTextLayout *layout, const QSizeF &size, QTextOption::WrapMode wordWrap,
                                   Qt::TextElideMode mode, qreal lineHeight, int flags, QStringList *lines,
                                   QPainter *painter, QPointF offset, const QColor &shadowColor, const QPointF &shadowOffset,
                                   const QBrush &background, qreal backgroundRadius, QList<QRectF> *boundingRegion)
{
    qreal height = 0;
    bool drawShadow = shadowColor.isValid();

    QString text = layout->engine()->hasFormats() ? layout->engine()->block.text() : layout->text();
    QTextOption &textOption = *const_cast<QTextOption *>(&layout->textOption());

    textOption.setWrapMode(wordWrap);

    if (flags & Qt::AlignRight)
        textOption.setAlignment(Qt::AlignRight);
    else if (flags & Qt::AlignHCenter)
        textOption.setAlignment(Qt::AlignHCenter);

    if (painter) {
        textOption.setTextDirection(painter->layoutDirection());
        layout->setFont(painter->font());
    } else {
        // dont paint
        layout->engine()->ignoreBidi = true;
    }

    layout->beginLayout();

    QTextLine line = layout->createLine();
    QRectF lastLineRect;
    while (line.isValid()) {
        height += lineHeight;
        if (height + lineHeight > size.height()) {
            const QString &endString = layout->engine()->elidedText(mode, qRound(size.width()), flags, line.textStart());

            layout->endLayout();
            layout->setText(endString);

            if (layout->engine()->block.docHandle())
                const_cast<QTextDocument *>(layout->engine()->block.document())->setPlainText(endString);

            textOption.setWrapMode(QTextOption::NoWrap);
            layout->beginLayout();
            line = layout->createLine();
            line.setLineWidth(size.width() - 1);
            text = endString;
        } else {
            line.setLineWidth(size.width());
        }

        line.setPosition(offset);

        const QRectF rect = QRectF(line.naturalTextRect().topLeft(), QSizeF(line.naturalTextRect().width(), lineHeight));

        if (painter) {
            if (background.style() != Qt::NoBrush)
                drawBackground(backgroundRadius, rect, lastLineRect, background, painter);

            if (drawShadow) {
                const QPen pen = painter->pen();

                painter->setPen(shadowColor);
                line.draw(painter, shadowOffset);
                // restore
                painter->setPen(pen);
            }

            line.draw(painter, QPointF(0, 0));
        }

        if (boundingRegion)
            boundingRegion->append(rect);

        offset.setY(offset.y() + lineHeight);

        if (lines)
            lines->append(text.mid(line.textStart(), line.textLength()));

        if (height + lineHeight > size.height())
            break;

        line = layout->createLine();
    }

    layout->endLayout();
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
