// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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
    void drawTextWithHighlight(QPainter *painter, const QTextLine &line, const QString &lineText, 
                              const QRectF &rect, int lineStartPos, const QList<QPair<int, int>> &allMatches);
    virtual void initLayoutOption(QTextLayout *lay);

private:
    // 查找文本中所有关键词匹配的位置
    QList<QPair<int, int>> findKeywordMatches(const QString &text) const;

    // 计算省略文本中的高亮匹配位置
    QList<QPair<int, int>> calculateElideHighlightMatches(
        const QString &elideText, 
        int elidePos, 
        Qt::TextElideMode elideMode, 
        const QList<QPair<int, int>> &originalMatches,
        int lineStartPos) const;

    // 确定省略位置 - 通过比较省略文本和原始文本
    int determineElidePosition(
        const QString &elideText,
        const QString &originalText, 
        Qt::TextElideMode elideMode) const;

protected:
    QTextDocument *document { nullptr };
    QMap<Attribute, QVariant> attributes {};

    QStringList highlightKeywords {};  // 需要高亮的关键字
    QColor highlightColor { QColor() };     // 高亮颜色
    bool enableHighlight { false };      // 是否启用高亮
};
}

#endif // ELIDETEXTLAYOUT_H
