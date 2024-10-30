// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    QList<QRectF> layout(const QRectF &rect, Qt::TextElideMode elideMode, QPainter *painter = nullptr, const QBrush &background = Qt::NoBrush, QStringList *textLines = nullptr);
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
