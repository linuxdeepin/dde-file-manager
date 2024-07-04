// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "delegatecommon.h"

#include <DArrowRectangle>
#include <DListView>
#include <DArrowRectangle>
#include <DPalette>

// #include <private/qtextengine_p.h>

DWIDGET_USE_NAMESPACE

namespace GlobalPrivate {

QRectF boundingRect(const QList<QRectF> &rects)
{
    QRectF bounding;

    if (rects.isEmpty())
        return bounding;

    bounding = rects.first();

    for (const QRectF &r : rects) {
        if (r.top() < bounding.top()) {
            bounding.setTop(r.top());
        }

        if (r.left() < bounding.left()) {
            bounding.setLeft(r.left());
        }

        if (r.right() > bounding.right()) {
            bounding.setRight(r.right());
        }

        if (r.bottom() > bounding.bottom()) {
            bounding.setBottom(r.bottom());
        }
    }

    return bounding;
}

QPainterPath boundingPath(QList<QRectF> rects, qreal radius, qreal padding)
{
    QPainterPath path;
    const QMarginsF margins(radius + padding, 0, radius + padding, 0);

    if (rects.count() == 1) {
        path.addRoundedRect(rects.first().marginsAdded(margins).adjusted(0, -padding, 0, padding), radius, radius);

        return path;
    }

    for (int i = 1; i < rects.count(); ++i) {
        if (qAbs(rects[i].x() - rects.at(i - 1).x()) < radius) {
            rects[i].setLeft(rects.at(i - 1).x());
        }

        if (qAbs(rects[i].right() - rects.at(i - 1).right()) < radius) {
            rects[i].setRight(rects.at(i - 1).right());
        }
    }

    const QRectF &first = rects.first().marginsAdded(margins);

    path.arcMoveTo(first.right() - radius * 2, first.y() - padding, radius * 2, radius * 2, 90);
    joinRightCorner(first, QRectF(), rects.at(1).marginsAdded(margins), radius, padding, &path);

    for (int i = 1; i < rects.count() - 1; ++i) {
        joinRightCorner(rects.at(i) + margins, rects.at(i - 1).marginsAdded(margins), rects.at(i + 1).marginsAdded(margins), radius, padding, &path);
    }

    QRectF last = rects.last();
    const QRectF &prevRect = rects.at(rects.count() - 2);

    joinRightCorner(last.marginsAdded(margins), prevRect.marginsAdded(margins), QRectF(), radius, padding, &path);
    joinLeftCorner(last.marginsAdded(margins), prevRect.marginsAdded(margins), QRectF(), radius, padding, &path);

    for (int i = rects.count() - 2; i > 0; --i)
        joinLeftCorner(rects.at(i) + margins, rects.at(i - 1).marginsAdded(margins), rects.at(i + 1).marginsAdded(margins), radius, padding, &path);

    joinLeftCorner(first, QRectF(), rects.at(1).marginsAdded(margins), radius, padding, &path);

    path.closeSubpath();

    return path;
}

void joinLeftCorner(const QRectF &rect, const QRectF &prevRect, const QRectF &nextRect,
                    const qreal &radius, const qreal &padding, QPainterPath *path)
{
    if (Q_LIKELY(prevRect.isValid())) {
        qreal newRadius = qMin(radius, qAbs(prevRect.right() - rect.right()) / 2);

        if (rect.right() > prevRect.right()) {
            path->arcTo(rect.right() - newRadius * 2, rect.y() - padding, newRadius * 2, newRadius * 2, 90, -90);
        } else if (rect.right() < prevRect.right()) {
            path->arcTo(rect.right(), rect.y() + padding, newRadius * 2, newRadius * 2, 90, 90);
        }
    } else {
        path->arcTo(rect.right() - radius * 2, rect.y() - padding, radius * 2, radius * 2, 90, -90);
    }

    if (Q_LIKELY(nextRect.isValid())) {
        qreal newRadius = qMin(radius, qAbs(nextRect.right() - rect.right()) / 2);

        if (rect.right() > nextRect.right()) {
            path->arcTo(rect.right() - newRadius * 2, rect.bottom() - newRadius * 2 + padding, newRadius * 2, newRadius * 2, 0, -90);
        } else if (rect.right() < nextRect.right()) {
            path->arcTo(rect.right(), rect.bottom() - newRadius * 2 - padding, newRadius * 2, newRadius * 2, 180, 90);
        }
    } else {
        path->arcTo(rect.right() - radius * 2, rect.bottom() - radius * 2 + padding, radius * 2, radius * 2, 0, -90);
    }
}

void joinRightCorner(const QRectF &rect, const QRectF &prevRect, const QRectF &nextRect,
                     const qreal &radius, const qreal &padding, QPainterPath *path)
{
    if (Q_LIKELY(prevRect.isValid())) {
        qreal newRadius = qMin(radius, qAbs(prevRect.right() - rect.right()) / 2);

        if (rect.right() > prevRect.right()) {
            path->arcTo(rect.right() - newRadius * 2, rect.y() - padding, newRadius * 2, newRadius * 2, 90, -90);
        } else if (rect.right() < prevRect.right()) {
            path->arcTo(rect.right(), rect.y() + padding, newRadius * 2, newRadius * 2, 90, 90);
        }
    } else {
        path->arcTo(rect.right() - radius * 2, rect.y() - padding, radius * 2, radius * 2, 90, -90);
    }

    if (Q_LIKELY(nextRect.isValid())) {
        qreal newRadius = qMin(radius, qAbs(nextRect.right() - rect.right()) / 2);

        if (rect.right() > nextRect.right()) {
            path->arcTo(rect.right() - newRadius * 2, rect.bottom() - newRadius * 2 + padding, newRadius * 2, newRadius * 2, 0, -90);
        } else if (rect.right() < nextRect.right()) {
            path->arcTo(rect.right(), rect.bottom() - newRadius * 2 - padding, newRadius * 2, newRadius * 2, 180, 90);
        }
    } else {
        path->arcTo(rect.right() - radius * 2, rect.bottom() - radius * 2 + padding, radius * 2, radius * 2, 0, -90);
    }
}

}   // namespace GlobalPrivate
