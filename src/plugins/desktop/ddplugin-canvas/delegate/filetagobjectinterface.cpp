/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "filetagobjectinterface.h"
#include "canvasitemdelegate.h"
#include "tagtextformat.h"

#include <QTextFormat>

DDP_CANVAS_USE_NAMESPACE

FileTagObjectInterface::FileTagObjectInterface(QObject *parent) : QObject(parent)
{

}

QSizeF FileTagObjectInterface::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(doc);
    Q_UNUSED(posInDocument);
    const TagTextFormat &f = static_cast<const TagTextFormat &>(format);
    const QList<QColor> &colors = f.colors();
    const int diameter = static_cast<int>(f.diameter());

    if (colors.size() == 1)
        return QSizeF(diameter, diameter);

    return QSizeF(diameter + (colors.size() - 1) * diameter / 2.0, diameter);
}

void FileTagObjectInterface::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                                        int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(doc);
    Q_UNUSED(posInDocument);
    const TagTextFormat &f = static_cast<const TagTextFormat &>(format);
    const QList<QColor> &colors = f.colors();
    const QColor borderColor = f.borderColor();
    qreal diameter = f.diameter();
    const qreal padding = diameter / 10.0;
    QRectF bounding_rect = rect.marginsRemoved(QMarginsF(padding, padding, padding, padding));
    diameter -= padding * 2;
    //CanvasItemDelegate::paintCircleList(painter, bounding_rect, diameter, colors, borderColor);
}
