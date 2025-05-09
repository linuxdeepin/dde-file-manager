// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elidetextlayout.h"

#include <QPainter>
#include <QtMath>
#include <QPainterPath>
#include <QTextDocument>
#include <QTextLayout>
#include <QTextBlock>
#include <QDebug>

#include <dfm-base/dfm_base_global.h>

using namespace dfmbase;

ElideTextLayout::ElideTextLayout(const QString &text)
    : document(new QTextDocument)
{
    document->setPlainText(text);

    attributes.insert(kFont, document->defaultFont());
    attributes.insert(kLineHeight, QFontMetrics(document->defaultFont()).height());
    attributes.insert(kBackgroundRadius, 0);
    attributes.insert(kAlignment, Qt::AlignHCenter);
    attributes.insert(kWrapMode, (uint)QTextOption::WrapAtWordBoundaryOrAnywhere);
    attributes.insert(kTextDirection, Qt::LeftToRight);
}

ElideTextLayout::~ElideTextLayout()
{
    delete document;
    document = nullptr;
}

void ElideTextLayout::setText(const QString &text)
{
    document->setPlainText(text);
}

QString ElideTextLayout::text() const
{
    return document->toPlainText();
}

QList<QPair<int, int>> ElideTextLayout::findKeywordMatches(const QString &text) const
{
    QList<QPair<int, int>> matches;
    
    // 查找所有关键词的匹配位置
    for (const QString &keyword : highlightKeywords) {
        if (keyword.isEmpty())
            continue;

        int startPos = 0;
        while (startPos < text.length()) {
            int keywordPos = text.indexOf(keyword, startPos, Qt::CaseInsensitive);
            if (keywordPos == -1)
                break;

            matches.append(qMakePair(keywordPos, keyword.length()));
            startPos = keywordPos + 1; // 允许重叠匹配
        }
    }
    
    // 按位置排序所有匹配
    std::sort(matches.begin(), matches.end());
    
    // 合并重叠的匹配区域
    for (int i = 0; i < matches.size() - 1; ) {
        if (matches[i].first + matches[i].second > matches[i+1].first) {
            int newEnd = std::max(matches[i].first + matches[i].second, 
                                matches[i+1].first + matches[i+1].second);
            matches[i].second = newEnd - matches[i].first;
            matches.removeAt(i+1);
        } else {
            i++;
        }
    }
    
    return matches;
}

int ElideTextLayout::determineElidePosition(const QString &elideText, const QString &originalText, Qt::TextElideMode elideMode) const
{
    // 如果两个文本相同，则没有省略发生
    if (elideText == originalText)
        return -1;
    
    // 标准省略符号
    QString ellipsis = "…";
    QString threeDotsEllipsis = "...";
    
    // 检查省略文本是否包含标准省略符号
    bool hasEllipsis = elideText.contains(ellipsis) || elideText.contains(threeDotsEllipsis);
    
    if (elideMode == Qt::ElideRight) {
        // 右侧省略：从前向后找到最后一个匹配的字符
        int i = 0;
        while (i < elideText.length() && i < originalText.length() && 
               elideText.at(i) == originalText.at(i)) {
            i++;
        }
        
        // 如果找到了省略符号，返回省略符号的位置
        if (hasEllipsis) {
            int ellipsisPos = elideText.indexOf(ellipsis, i - 1);
            if (ellipsisPos != -1)
                return ellipsisPos;
                
            ellipsisPos = elideText.indexOf(threeDotsEllipsis, i - 1);
            if (ellipsisPos != -1)
                return ellipsisPos;
        }
        
        // 没有找到明确的省略符号，返回不匹配的起始位置
        return i;
    } 
    else if (elideMode == Qt::ElideLeft) {
        // 左侧省略：从后向前找到最后一个匹配的字符
        int elideLen = elideText.length();
        int originalLen = originalText.length();
        int i = 0;
        
        while (i < elideLen && i < originalLen && 
               elideText.at(elideLen - 1 - i) == originalText.at(originalLen - 1 - i)) {
            i++;
        }
        
        // 如果找到了省略符号，返回省略符号后的位置
        if (hasEllipsis) {
            int ellipsisPos = elideText.indexOf(ellipsis);
            if (ellipsisPos != -1)
                return ellipsisPos + ellipsis.length();
                
            ellipsisPos = elideText.indexOf(threeDotsEllipsis);
            if (ellipsisPos != -1)
                return ellipsisPos + threeDotsEllipsis.length();
        }
        
        // 没有找到明确的省略符号，返回不匹配的起始位置
        return elideLen - i;
    } 
    else if (elideMode == Qt::ElideMiddle) {
        // 中间省略：需要从两端开始匹配
        int leftMatchLen = 0;  // 左侧匹配长度
        int rightMatchLen = 0; // 右侧匹配长度
        
        // 计算左侧匹配长度
        while (leftMatchLen < elideText.length() && leftMatchLen < originalText.length() && 
               elideText.at(leftMatchLen) == originalText.at(leftMatchLen)) {
            leftMatchLen++;
        }
        
        // 计算右侧匹配长度
        int elideLen = elideText.length();
        int originalLen = originalText.length();
        while (rightMatchLen < elideLen - leftMatchLen && rightMatchLen < originalLen - leftMatchLen && 
               elideText.at(elideLen - 1 - rightMatchLen) == originalText.at(originalLen - 1 - rightMatchLen)) {
            rightMatchLen++;
        }
        
        // 如果找到了省略符号，返回省略符号的位置
        if (hasEllipsis) {
            int ellipsisPos = elideText.indexOf(ellipsis, leftMatchLen);
            if (ellipsisPos != -1 && ellipsisPos <= elideLen - rightMatchLen)
                return ellipsisPos;
                
            ellipsisPos = elideText.indexOf(threeDotsEllipsis, leftMatchLen);
            if (ellipsisPos != -1 && ellipsisPos <= elideLen - rightMatchLen)
                return ellipsisPos;
        }
        
        // 没有找到明确的省略符号，返回左侧匹配的末尾位置
        return leftMatchLen;
    } 
    
    // 默认情况（不应该到达这里）
    return 0;
}

QList<QPair<int, int>> ElideTextLayout::calculateElideHighlightMatches(
    const QString &elideText, 
    int elidePos, 
    Qt::TextElideMode elideMode, 
    const QList<QPair<int, int>> &originalMatches,
    int lineStartPos) const
{
    QList<QPair<int, int>> matches;
    
    // 首先处理完整匹配 - 在省略后的文本中查找关键词
    matches = findKeywordMatches(elideText);
    
    // 特殊处理：检查被省略号截断的关键词部分
    if (elideMode == Qt::ElideRight && elidePos > 0) {
        // 右侧省略：检查每个原始匹配项是否在省略处被截断
        for (const auto &match : originalMatches) {
            int matchStart = match.first;
            int matchLength = match.second;
            
            // 计算在省略前文本(elideText)中对应的位置
            int elideTextStartPos = lineStartPos;
            
            // 如果匹配项开始在可见区域内但结束在省略区域
            if (matchStart >= elideTextStartPos && matchStart < elideTextStartPos + elidePos &&
                matchStart + matchLength > elideTextStartPos + elidePos) {
                // 只取截断前的可见部分
                int visibleLength = elidePos - (matchStart - elideTextStartPos);
                if (visibleLength > 0) {
                    // 匹配项在省略处被截断，添加可见部分
                    int adjustedStart = matchStart - elideTextStartPos;
                    matches.append(qMakePair(adjustedStart, visibleLength));
                }
            }
        }
    } else if (elideMode == Qt::ElideLeft && elidePos > 0) {
        // 左侧省略：检查每个原始匹配项是否在省略处被截断
        int originalLineStart = lineStartPos;
        int visibleStartInOriginal = originalLineStart + elidePos + 1;
        
        for (const auto &match : originalMatches) {
            int matchStart = match.first;
            int matchEnd = matchStart + match.second;
            
            // 如果匹配项开始在省略区域但结束在可见区域
            if (matchStart < visibleStartInOriginal && matchEnd > visibleStartInOriginal) {
                // 计算可见部分的长度
                int visibleLength = matchEnd - visibleStartInOriginal;
                // 调整到省略后文本中的位置
                matches.append(qMakePair(elidePos, visibleLength));
            }
        }
    } else if (elideMode == Qt::ElideMiddle && elidePos > 0) {
        // 计算在原始文本中前半部分对应的区域
        int originalLineStart = lineStartPos;
        int firstVisiblePartLength = elidePos;
        
        // 估算原始文本中省略的部分长度（这是近似值）
        int ellipsisInOriginalPos = originalLineStart + firstVisiblePartLength;
        
        // 检查每个原始匹配
        for (const auto &match : originalMatches) {
            int matchStart = match.first;
            int matchEnd = matchStart + match.second;
            
            // 检查是否跨越了前半部分的末尾
            if (matchStart < ellipsisInOriginalPos && matchEnd > ellipsisInOriginalPos) {
                // 添加前半部分的可见匹配
                int visibleLengthInFirstPart = ellipsisInOriginalPos - matchStart;
                if (visibleLengthInFirstPart > 0) {
                    matches.append(qMakePair(matchStart - originalLineStart, visibleLengthInFirstPart));
                }
                
                // 这里我们不添加后半部分，因为难以准确计算位置映射
                // 如果需要，可以在详细了解Qt的elide实现后添加更精确的处理
            }
        }
    }
    
    // 对匹配进行排序和合并
    std::sort(matches.begin(), matches.end());
    
    for (int i = 0; i < matches.size() - 1; ) {
        if (matches[i].first + matches[i].second > matches[i+1].first) {
            int newEnd = std::max(matches[i].first + matches[i].second, 
                              matches[i+1].first + matches[i+1].second);
            matches[i].second = newEnd - matches[i].first;
            matches.removeAt(i+1);
        } else {
            i++;
        }
    }
    
    return matches;
}

QList<QRectF> ElideTextLayout::layout(const QRectF &rect, Qt::TextElideMode elideMode, QPainter *painter, const QBrush &background, QStringList *textLines)
{
    QList<QRectF> ret;
    QTextLayout *lay = document->firstBlock().layout();
    if (!lay) {
        qCWarning(logDFMBase) << "invaild block" << document->firstBlock().text();
        return ret;
    }

    initLayoutOption(lay);
    int textLineHeight = attribute<int>(kLineHeight);
    QSizeF size = rect.size();
    QPointF offset = rect.topLeft();
    qreal curHeight = 0;

    // for draw background.
    QRectF lastLineRect;
    QString elideText;
    QString curText = text();
    bool paintLineWithHighlight = enableHighlight && highlightColor.isValid() && !highlightKeywords.isEmpty();
    
    // 预处理整个文本中的所有关键词匹配位置
    QList<QPair<int, int>> allMatches; // 保存所有匹配位置: <开始位置, 长度>
    if (paintLineWithHighlight) {
        allMatches = findKeywordMatches(curText);
    }
    
    // 一个更新后的匹配列表，用于处理elideText情况
    QList<QPair<int, int>> currentMatches = allMatches;

    auto processLine = [this, &ret, painter, &lastLineRect, background, textLineHeight, &curText, textLines, 
                        paintLineWithHighlight, &currentMatches](QTextLine &line) {
        QRectF lRect = line.naturalTextRect();
        lRect.setHeight(textLineHeight);

        ret.append(lRect);
        if (textLines) {
            const auto &t = curText.mid(line.textStart(), line.textLength());
            textLines->append(t);
        }

        // draw
        if (painter) {
            // draw background
            if (background.style() != Qt::NoBrush) {
                lastLineRect = drawLineBackground(painter, lRect, lastLineRect, background);
            }
            
            // 获取当前行的文本范围
            int lineStart = line.textStart();
            int lineEnd = lineStart + line.textLength();
            
            // 检查当前行是否需要高亮显示（检查是否有任何关键词与当前行有重叠）
            bool needHighlight = false;
            if (paintLineWithHighlight) {
                for (const auto &match : currentMatches) {
                    int matchStart = match.first;
                    int matchEnd = matchStart + match.second;
                    
                    // 如果匹配区域与当前行有任何重叠
                    if (matchEnd > lineStart && matchStart < lineEnd) {
                        needHighlight = true;
                        break;
                    }
                }
            }

            if (!paintLineWithHighlight || !needHighlight) {
                // draw text line without highlight
                line.draw(painter, QPoint(0, 0));
                return;
            }

            // 获取当前行文本
            QString lineText = curText.mid(lineStart, line.textLength());
            drawTextWithHighlight(painter, line, lineText, lRect, lineStart, currentMatches);
        }
    };

    {
        lay->beginLayout();
        QTextLine line = lay->createLine();
        while (line.isValid()) {
            curHeight += textLineHeight;
            line.setLineWidth(size.width());
            line.setPosition(offset);

            // check next line is out or not.
            if (curHeight + textLineHeight > size.height()) {
                auto nextLine = lay->createLine();
                if (nextLine.isValid()) {
                    // elide current line.
                    QFontMetrics fm(lay->font());
                    QString originalText = text().mid(line.textStart());
                    elideText = fm.elidedText(originalText, elideMode, qRound(size.width()));
                    
                    // 判断文本是否被省略以及省略位置
                    bool isElided = elideText != originalText;
                    int elidePos = 0;
                    
                    if (isElided) {
                        // 使用改进的方法确定省略位置
                        elidePos = determineElidePosition(elideText, originalText, elideMode);
                        if (elidePos < 0) // 如果没有省略（虽然不太可能发生）
                            elidePos = 0;
                    }
                    
                    curText = elideText;
                    
                    // 如果有省略，需要处理高亮匹配位置
                    if (isElided && paintLineWithHighlight) {
                        // 重新计算省略文本中的高亮匹配位置
                        currentMatches = calculateElideHighlightMatches(
                            elideText, 
                            elidePos, 
                            elideMode, 
                            allMatches, 
                            line.textStart()
                        );
                    }
                    break;
                }
                // next line is empty.
            }

            processLine(line);

            // next line
            line = lay->createLine();
            offset.setY(offset.y() + textLineHeight);
        }

        lay->endLayout();
    }

    // process last elided line.
    if (!elideText.isEmpty()) {
        QTextLayout newlay;
        newlay.setFont(lay->font());
        {
            auto oldWrap = static_cast<QTextOption::WrapMode>(attribute<uint>(kWrapMode));
            setAttribute(kWrapMode, static_cast<uint>(QTextOption::NoWrap));
            initLayoutOption(&newlay);

            // restore
            setAttribute(kWrapMode, oldWrap);
        }

        newlay.setText(elideText);
        newlay.beginLayout();
        auto line = newlay.createLine();
        line.setLineWidth(size.width() - 1);
        line.setPosition(offset);

        processLine(line);
        newlay.endLayout();
    }

    return ret;
}

QRectF ElideTextLayout::drawLineBackground(QPainter *painter, const QRectF &curLineRect, QRectF lastLineRect, const QBrush &brush) const
{
    const qreal backgroundRadius = attribute<qreal>(kBackgroundRadius);
    const QMarginsF margins(backgroundRadius, 0, backgroundRadius, 0);
    QRectF backBounding = curLineRect;
    QPainterPath path;

    if (lastLineRect.isValid()) {
        if (qAbs(curLineRect.width() - lastLineRect.width()) < backgroundRadius * 2) {
            backBounding.setWidth(lastLineRect.width());
            backBounding.moveCenter(curLineRect.center());
            path.moveTo(lastLineRect.x() - backgroundRadius, lastLineRect.bottom() - backgroundRadius);
            path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius);
            path.lineTo(lastLineRect.right() + backgroundRadius, backBounding.bottom() - backgroundRadius);
            path.arcTo(backBounding.right() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 0, -90);
            path.lineTo(backBounding.x(), backBounding.bottom());
            path.arcTo(backBounding.x() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, -90);
            lastLineRect = backBounding;
        } else if (lastLineRect.width() > curLineRect.width()) {
            backBounding += margins;
            path.moveTo(backBounding.x() - backgroundRadius, backBounding.y() - 1);
            path.arcTo(backBounding.x() - backgroundRadius * 2, backBounding.y() - 1, backgroundRadius * 2, backgroundRadius * 2 + 1, 90, -90);
            path.lineTo(backBounding.x(), backBounding.bottom() - backgroundRadius);
            path.arcTo(backBounding.x(), backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 180, 90);
            path.lineTo(backBounding.right() - backgroundRadius, backBounding.bottom());
            path.arcTo(backBounding.right() - backgroundRadius * 2, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, 90);
            path.lineTo(backBounding.right(), backBounding.top() + backgroundRadius);
            path.arcTo(backBounding.right(), backBounding.top() - 1, backgroundRadius * 2, backgroundRadius * 2 + 1, 180, -90);
            path.closeSubpath();
            lastLineRect = curLineRect;
        } else {
            backBounding += margins;
            path.moveTo(lastLineRect.x() - backgroundRadius * 2, lastLineRect.bottom());
            path.arcTo(lastLineRect.x() - backgroundRadius * 3, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, 90);
            path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2);
            path.arcTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 180, 90);
            path.addRoundedRect(backBounding, backgroundRadius, backgroundRadius);
            lastLineRect = curLineRect;
        }
    } else {
        lastLineRect = backBounding;
        path.addRoundedRect(backBounding + margins, backgroundRadius, backgroundRadius);
    }

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setOpacity(1);
    painter->fillPath(path, brush);

    painter->restore();
    return lastLineRect;
}

void ElideTextLayout::drawTextWithHighlight(QPainter *painter, const QTextLine &line, const QString &lineText, 
                                           const QRectF &rect, int lineStartPos, const QList<QPair<int, int>> &allMatches)
{
    // 1. 先绘制整行普通文本
    painter->drawText(rect, lineText, QTextOption(document->defaultTextOption().alignment()));
    
    // 2. 仅在匹配区域上绘制高亮文本
    if (!allMatches.isEmpty()) {
        painter->save();
        painter->setPen(highlightColor);
        
        int lineEnd = lineStartPos + lineText.length();
        
        for (const auto &match : allMatches) {
            int matchStart = match.first;
            int matchEnd = matchStart + match.second;
            
            // 跳过与当前行无关的匹配
            if (matchEnd <= lineStartPos || matchStart >= lineEnd)
                continue;
            
            // 计算此行中要高亮的部分
            int highlightStart = qMax(matchStart, lineStartPos) - lineStartPos;
            int highlightEnd = qMin(matchEnd, lineEnd) - lineStartPos;
            int highlightLength = highlightEnd - highlightStart;
            
            if (highlightLength <= 0)
                continue;
            
            // 计算高亮区域在当前行的X坐标位置
            qreal keywordXPos = line.cursorToX(lineStartPos + highlightStart) - line.cursorToX(lineStartPos);
            qreal keywordWidth = line.cursorToX(lineStartPos + highlightStart + highlightLength) 
                               - line.cursorToX(lineStartPos + highlightStart);
            
            // 绘制高亮文本
            QRectF highlightRect(rect.x() + keywordXPos, rect.y(), keywordWidth, rect.height());
            painter->drawText(highlightRect,
                             lineText.mid(highlightStart, highlightLength),
                             QTextOption(document->defaultTextOption().alignment()));
        }
        
        painter->restore();
    }
}

void ElideTextLayout::initLayoutOption(QTextLayout *lay)
{
    auto opt = lay->textOption();
    opt.setAlignment((Qt::Alignment)attribute<uint>(kAlignment));
    opt.setWrapMode((QTextOption::WrapMode)attribute<uint>(kWrapMode));
    opt.setTextDirection(attribute<Qt::LayoutDirection>(kTextDirection));
    lay->setTextOption(opt);
    lay->setFont(attribute<QFont>(kFont));
}
