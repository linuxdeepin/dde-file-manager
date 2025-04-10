// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ELIDETEXTLAYOUT_H
#define ELIDETEXTLAYOUT_H

#include <QString>
#include <QBrush>
#include <QVariant>
#include <QTextLine>

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

    // 设置高亮关键字
    inline void setHighlightKeywords(const QStringList &keywords) {
        highlightKeywords.append(keywords);
    }

    // 设置高亮颜色
    inline void setHighlightColor(const QColor &color) {
        highlightColor = color;
    }

    // 启用或禁用高亮
    inline void setHighlightEnabled(bool enable) {
        enableHighlight = enable;
    }

protected:
    QRectF drawLineBackground(QPainter *painter, const QRectF &curLineRect, QRectF lastLineRect, const QBrush &brush) const;
    void drawTextWithHighlight(QPainter *painter, const QTextLine &line, const QString &lineText, const QRectF &rect);
    virtual void initLayoutOption(QTextLayout *lay);
protected:
    QTextDocument *document { nullptr };
    QMap<Attribute, QVariant> attributes {};

    QStringList highlightKeywords {};  // 需要高亮的关键字
    QColor highlightColor { QColor() };     // 高亮颜色
    bool enableHighlight { false };      // 是否启用高亮
};
}

#endif // ELIDETEXTLAYOUT_H
