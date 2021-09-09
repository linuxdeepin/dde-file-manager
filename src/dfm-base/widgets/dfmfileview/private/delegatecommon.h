/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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

const int ICON_SPACING = 16;
const int ICON_MODE_RECT_RADIUS = 4;
const int TEXT_PADDING = 4;
const int ICON_MODE_ICON_SPACING = 5;
const int ICON_MODE_BACK_RADIUS = 18;
const int COLUMU_PADDING = 10;
const int LIST_MODE_RECT_RADIUS = 8;
const int LIST_EDITER_HEIGHT = 22;
const int LIST_MODE_EDITOR_LEFT_PADDING = -3;
const int LIST_VIEW_ICON_SIZE = 24;
const int LIST_MODE_LEFT_MARGIN = 20;
const int LIST_MODE_RIGHT_MARGIN = 20;
const int LEFT_PADDING = 10;
const int RIGHT_PADDING = 10;
static QString replaceChars = "";

QRectF boundingRect(const QList<QRectF> &rects);
QPainterPath boundingPath(QList<QRectF> rects, qreal radius, qreal padding);
static void elideText(QTextLayout *layout, const QSizeF &size,
                      QTextOption::WrapMode wordWrap,
                      Qt::TextElideMode mode, qreal lineHeight,
                      int flags = 0, QStringList *lines = nullptr,
                      QPainter *painter = nullptr, QPointF offset = QPoint(0, 0),
                      const QColor &shadowColor = QColor(),
                      const QPointF &shadowOffset = QPointF(0, 1),
                      const QBrush &background = QBrush(Qt::NoBrush),
                      qreal backgroundRadius = 4,
                      QList<QRectF> *boundingRegion = nullptr);
QString elideText(const QString &text, const QSizeF &size,
                  QTextOption::WrapMode wordWrap, const QFont &font,
                  Qt::TextElideMode mode, qreal lineHeight, qreal flags = 0);

QString replaceFileName(QString name, QString chars = replaceChars);

void setFileNameReplaceChars(const QString &rep);

QString fileNameReplaceChars();

void showAlertMessage(QPoint globalPoint, const QColor &backgroundColor,
                      const QString &text, int duration = 3000);

} //namespace GlobalPrivate

#endif // DELEGATECOMMON_H
