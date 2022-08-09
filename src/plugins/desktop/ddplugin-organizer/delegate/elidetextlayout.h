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
#ifndef ELIDETEXTLAYOUT_H
#define ELIDETEXTLAYOUT_H

#include "ddplugin_organizer_global.h"

#include <QTextLayout>

namespace ddplugin_organizer {

class ElideTextLayout : public QTextLayout
{
public:
    using QTextLayout::QTextLayout;
    void setLineHeight(int height);
    QList<QRectF> layout(const QRectF &rect, Qt::TextElideMode mode, QPainter *painter = nullptr, const QBrush &background = Qt::NoBrush);
    QTextOption::WrapMode wrapMode() const;
    void setWrapMode(QTextOption::WrapMode);
    void setAlignment(Qt::Alignment);
    void setBackgroundRadius(qreal radius);
    void setTextDirection(Qt::LayoutDirection direction);
    Qt::LayoutDirection textDirection () const;
public:
    inline Qt::Alignment alignment() const  {
        return alignmentFlags;
    }
    inline int lineHeight() const {
        return textLineHeight;
    }

    inline qreal backgroundRadius() const {
        return lineRadius;
    }
protected:
    inline QRectF naturalTextRect(QRectF rect) {
        rect.setHeight(textLineHeight);
        return rect;
    }
private:
    QRectF drawLineBackground(QPainter *painter, const QRectF &curLineRect, QRectF lastLineRect, const QBrush &brush) const;
private:
    int textLineHeight = 1;
    qreal lineRadius = 0;
    Qt::Alignment alignmentFlags;
};

}

#endif // ELIDETEXTLAYOUT_H
