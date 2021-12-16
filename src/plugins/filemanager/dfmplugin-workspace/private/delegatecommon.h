/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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

const int kIconSpacing = 16;
const int kIconModeRectRadius = 4;
const int kTextPadding = 4;
const int kIconModeIconSpacing = 5;
const int kIconModeBackRadius = 18;
const int kColumnPadding = 10;
const int kListModeRectRadius = 8;
const int kListEditorHeight = 22;
const int kListModeEditorLeftPadding = -3;
const int kListViewIconSize = 24;
const int kListModeLeftMargin = 20;
const int kListModeRightMargin = 20;
const int kLeftPadding = 10;
const int kRightPadding = 10;
static QString replaceChars = "";

void joinLeftCorner(const QRectF &rect, const QRectF &prevRect, const QRectF &nextRect,
                    const qreal &radius, const qreal &padding, QPainterPath *path);
void joinRightCorner(const QRectF &rect, const QRectF &prevRect, const QRectF &nextRect,
                     const qreal &radius, const qreal &padding, QPainterPath *path);
QRectF boundingRect(const QList<QRectF> &rects);
QPainterPath boundingPath(QList<QRectF> rects, qreal radius, qreal padding);
QString replaceFileName(QString name, QString chars = replaceChars);
void setFileNameReplaceChars(const QString &rep);
QString fileNameReplaceChars();
void showAlertMessage(QPoint globalPoint, const QColor &backgroundColor,
                      const QString &text, int duration = 3000);

}   // namespace GlobalPrivate

#endif   // DELEGATECOMMON_H
