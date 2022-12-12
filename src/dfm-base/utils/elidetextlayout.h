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

#include <QString>
#include <QBrush>
#include <QVariant>

class QPainter;
class QTextDocument;
class QTextLayout;

namespace dfmbase {

class ElideTextLayout
{
public:
    enum Attribute {
        kLineHeight = 0,
        kAlignment,
        kBackgroundRadius,
        kWrapMode,
        kTextDirection,
        kFont
    };
public:
    explicit ElideTextLayout(const QString &text = "");
    virtual ~ElideTextLayout();
    void setText(const QString &text);
    QString text() const;
    QList<QRectF> layout(const QRectF &rect, Qt::TextElideMode elideMode, QPainter *painter = nullptr, const QBrush &background = Qt::NoBrush);
public:
    inline QTextDocument *documentHandle() {
        return document;
    }

    inline void setAttribute(Attribute attr, const QVariant &value) {
        attributes.insert(attr, value);
    }

    template<typename T>
    inline T attribute(Attribute attr) const  {
        return attributes.value(attr).value<T>();
    }

protected:
    QRectF drawLineBackground(QPainter *painter, const QRectF &curLineRect, QRectF lastLineRect, const QBrush &brush) const;
    virtual void initLayoutOption(QTextLayout *lay);
protected:
    QTextDocument *document = nullptr;
    QMap<Attribute, QVariant> attributes;
};
}

#endif // ELIDETEXTLAYOUT_H
