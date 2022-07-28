/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "elidetextlayout.h"

#include <QtMath>
#include <QPainterPath>

#include <private/qtextengine_p.h>

void ElideTextLayout::setLineHeight(int height)
{
    textLineHeight = height;
}

QList<QRectF> ElideTextLayout::layout(const QRectF &rect, Qt::TextElideMode mode, QPainter *painter, const QBrush &background)
{
    QList<QRectF> ret;

    // dont paint
    engine()->ignoreBidi = !painter;

    QSizeF size = rect.size();
    QPointF offset = rect.topLeft();

    QString curText = engine()->hasFormats() ? engine()->block.text() : text();
    beginLayout();

    QTextLine line = createLine();
    qreal height = 0;
    // for draw background.
    QRectF lastLineRect;

    auto oldWrap = wrapMode();
    while (line.isValid()) {
        height += textLineHeight;
        if (height + textLineHeight > size.height()) {
            const QString &end_str = engine()->elidedText(mode, qRound(size.width()), alignment(), line.textStart());
            endLayout();
            setText(end_str);

            if (engine()->block.docHandle())
                const_cast<QTextDocument *>(engine()->block.document())->setPlainText(end_str);

            // use NoWrap
            setWrapMode(QTextOption::NoWrap);
            beginLayout();
            line = createLine();
            line.setLineWidth(size.width() - 1);
            curText = end_str;
        } else {
            line.setLineWidth(size.width());
        }

        line.setPosition(offset);

        const QRectF lRect = naturalTextRect(line.naturalTextRect());

        // draw
        if (painter) {
            // draw background
            if (background.style() != Qt::NoBrush) {
                lastLineRect = drawLineBackground(painter, lRect, lastLineRect, background);
            }

            // draw text line
            line.draw(painter, QPoint(0, 0));
        }

        ret.append(lRect);

        offset.setY(offset.y() + textLineHeight);
        if (height + textLineHeight > size.height())
            break;

        line = createLine();
    }

    // restore
    setWrapMode(oldWrap);

    return ret;
}

QTextOption::WrapMode ElideTextLayout::wrapMode() const
{
    return textOption().wrapMode();
}

void ElideTextLayout::setAlignment(Qt::Alignment flags)
{
    auto opt = textOption();

    if (flags & Qt::AlignRight)
        opt.setAlignment(Qt::AlignRight);
    else if (flags & Qt::AlignHCenter)
        opt.setAlignment(Qt::AlignHCenter);

    setTextOption(opt);

    alignmentFlags = flags;
}

void ElideTextLayout::setBackgroundRadius(qreal radius)
{
    lineRadius = radius;
}

void ElideTextLayout::setTextDirection(Qt::LayoutDirection direction)
{
    auto opt = textOption();
    opt.setTextDirection(direction);
    setTextOption(opt);
}

Qt::LayoutDirection ElideTextLayout::textDirection() const
{
    return textOption().textDirection();
}

QRectF ElideTextLayout::drawLineBackground(QPainter *painter, const QRectF &curLineRect, QRectF lastLineRect, const QBrush &brush) const
{
    const qreal backgroundRadius = lineRadius;
    const QMarginsF margins(backgroundRadius, 0, backgroundRadius, 0);
    QRectF backBounding = curLineRect;
    QPainterPath path;

    if (lastLineRect.isValid()) {
        if (qAbs(curLineRect.width() - lastLineRect.width()) < backgroundRadius * 2) {
            backBounding.setWidth(lastLineRect.width());
            backBounding.moveCenter(curLineRect.center());
            path.moveTo(lastLineRect.x() - backgroundRadius, lastLineRect.bottom() - backgroundRadius);
            path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius);
            path.lineTo(lastLineRect.right() + backgroundRadius, backBounding.bottom() - backgroundRadius);
            path.arcTo(backBounding.right() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 0, -90);
            path.lineTo(backBounding.x(), backBounding.bottom());
            path.arcTo(backBounding.x() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, -90);
            lastLineRect = backBounding;
        } else if (lastLineRect.width() > curLineRect.width()) {
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
            lastLineRect = curLineRect;
        } else {
            backBounding += margins;
            path.moveTo(lastLineRect.x() - backgroundRadius * 2, lastLineRect.bottom());
            path.arcTo(lastLineRect.x() - backgroundRadius * 3, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, 90);
            path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2);
            path.arcTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 180, 90);
            path.addRoundedRect(backBounding, backgroundRadius, backgroundRadius);
            lastLineRect = curLineRect;
        }
    } else {
        lastLineRect = backBounding;
        path.addRoundedRect(backBounding + margins, backgroundRadius, backgroundRadius);
    }

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setOpacity(1);
    painter->fillPath(path, brush);

    painter->restore();
    return lastLineRect;
}

void ElideTextLayout::setWrapMode(QTextOption::WrapMode mode)
{
    auto opt = textOption();
    opt.setWrapMode(mode);
    setTextOption(opt);
}

