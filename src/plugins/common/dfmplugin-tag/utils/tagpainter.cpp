// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagpainter.h"
#include "tagtextformat.h"
#include "taghelper.h"

#include <QPainter>

DPTAG_USE_NAMESPACE

TagPainter::TagPainter()
    : QObject()
{
}

QSizeF TagPainter::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(posInDocument)
    Q_UNUSED(doc)

    const TagTextFormat &f = static_cast<const TagTextFormat &>(format);
    const QList<QColor> &colors = f.colors();
    const double diameter = f.diameter();

    if (colors.size() == 1)
        return QSizeF(diameter, diameter);

    return QSizeF(diameter + (colors.size() - 1) * diameter / 2.0, diameter);
}

void TagPainter::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(posInDocument)
    Q_UNUSED(doc)

    const TagTextFormat &f = static_cast<const TagTextFormat &>(format);
    const QList<QColor> &colors = f.colors();
    qreal diameter = f.diameter();
    const qreal padding = diameter / 10.0;
    QRectF boundingRect = rect;
    boundingRect.moveLeft(rect.left() - padding);

    TagHelper::instance()->paintTags(painter, boundingRect, colors);
}
