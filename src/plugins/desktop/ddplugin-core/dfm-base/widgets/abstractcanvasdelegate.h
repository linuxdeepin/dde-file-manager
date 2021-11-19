/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#ifndef ABSTRACTCANVASDELEGATE_H
#define ABSTRACTCANVASDELEGATE_H

#include "dfm-base/dfm_base_global.h"

#include <QStyledItemDelegate>
#include <QTextOption>
#include <QTextLayout>

DFMBASE_BEGIN_NAMESPACE
class AbstractCanvas;
class AbstractCanvasDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit AbstractCanvasDelegate(AbstractCanvas *parent = nullptr);
    ~AbstractCanvasDelegate() {}

public:
    static void paintCircleList(QPainter *painter, QRectF boundingRect, qreal diameter, const QList<QColor> &colors, const QColor &borderColor);
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
};
DFMBASE_END_NAMESPACE

#endif   // ABSTRACTCANVASDELEGATE_H
