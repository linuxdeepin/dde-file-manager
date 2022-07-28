/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
