// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DELEGATECOMMON_H
#define DELEGATECOMMON_H

#include <QRect>
#include <QPainterPath>
#include <QTextLayout>

namespace GlobalPrivate {

inline constexpr int kIconSpacing { 16 };
inline constexpr int kIconModeRectRadius { 4 };
inline constexpr int kTextPadding { 4 };
inline constexpr int kIconModeIconSpacing { 5 };
inline constexpr int kIconModeBackRadius { 18 };
inline constexpr int kListModeRectRadius { 8 };
inline constexpr int kListEditorHeight { 22 };
inline constexpr int kListModeEditorLeftPadding { -3 };

void joinLeftCorner(const QRectF &rect, const QRectF &prevRect, const QRectF &nextRect,
                    const qreal &radius, const qreal &padding, QPainterPath *path);
void joinRightCorner(const QRectF &rect, const QRectF &prevRect, const QRectF &nextRect,
                     const qreal &radius, const qreal &padding, QPainterPath *path);
QRectF boundingRect(const QList<QRectF> &rects);
QPainterPath boundingPath(QList<QRectF> rects, qreal radius, qreal padding);

}   // namespace GlobalPrivate

#endif   // DELEGATECOMMON_H
