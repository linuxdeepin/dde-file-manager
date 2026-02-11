// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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

    // 统一：将原始匹配区间与“原文可见区间”相交，映射到 elideText 中
    auto mapOverlaps = [&](int visibleStartInOriginal, int visibleEndInOriginal, int mappedStartInElide) {
        for (const auto &match : originalMatches) {
            int matchStart = match.first;
            int matchEnd = matchStart + match.second;
            int isectStart = qMax(matchStart, visibleStartInOriginal);
            int isectEnd = qMin(matchEnd, visibleEndInOriginal);
            if (isectEnd > isectStart) {
                int length = isectEnd - isectStart;
                int startInElide = mappedStartInElide + (isectStart - visibleStartInOriginal);
                matches.append(qMakePair(startInElide, length));
            }
        }
    };

    if (elidePos > 0) {
        const QString fullText = text();
        const int originalLineLength = fullText.length() - lineStartPos;

        const QString ellipsis = "…";
        const QString threeDots = "...";
        int ellipsisStart = -1;
        int ellipsisLen = 0;

        if (elideMode == Qt::ElideRight) {
            // 左侧可见，后跟省略号
            ellipsisStart = elideText.indexOf(ellipsis, qMax(0, elidePos - 1));
            if (ellipsisStart >= 0) ellipsisLen = ellipsis.length();
            if (ellipsisStart < 0) {
                ellipsisStart = elideText.indexOf(threeDots, qMax(0, elidePos - 1));
                if (ellipsisStart >= 0) ellipsisLen = threeDots.length();
            }
            if (ellipsisStart < 0) { ellipsisStart = elidePos; ellipsisLen = 0; }

            int leftVisibleLen = ellipsisStart;
            mapOverlaps(lineStartPos, lineStartPos + leftVisibleLen, 0);
        } else if (elideMode == Qt::ElideLeft) {
            // 省略号后是右侧可见
            ellipsisStart = elideText.indexOf(ellipsis);
            if (ellipsisStart == 0) ellipsisLen = ellipsis.length();
            if (ellipsisLen == 0) {
                int p = elideText.indexOf(threeDots);
                if (p == 0) ellipsisLen = threeDots.length();
            }
            if (ellipsisLen == 0) { ellipsisLen = 1; }

            int rightVisibleLen = elideText.length() - ellipsisLen;
            int visibleStartInOriginal = lineStartPos + originalLineLength - rightVisibleLen;
            int visibleEndInOriginal = lineStartPos + originalLineLength;
            mapOverlaps(visibleStartInOriginal, visibleEndInOriginal, ellipsisLen);
        } else if (elideMode == Qt::ElideMiddle) {
            // 前半段 + 省略号 + 后半段
            ellipsisStart = elideText.indexOf(ellipsis, elidePos);
            if (ellipsisStart >= 0) ellipsisLen = ellipsis.length();
            if (ellipsisStart < 0) {
                ellipsisStart = elideText.indexOf(threeDots, elidePos);
                if (ellipsisStart >= 0) ellipsisLen = threeDots.length();
            }
            if (ellipsisStart < 0) { ellipsisStart = elidePos; ellipsisLen = 1; }

            int leftVisibleLen = ellipsisStart;
            int rightVisibleLen = elideText.length() - (ellipsisStart + ellipsisLen);

            // 前半段映射到 [0, leftVisibleLen)
            mapOverlaps(lineStartPos, lineStartPos + leftVisibleLen, 0);
            // 后半段映射到 [ellipsisStart+ellipsisLen, end)
            int rightStartInOriginal = lineStartPos + originalLineLength - rightVisibleLen;
            mapOverlaps(rightStartInOriginal, rightStartInOriginal + rightVisibleLen, ellipsisStart + ellipsisLen);
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
    if (allMatches.isEmpty()) {
        // 如果没有匹配项，直接绘制普通文本
        line.draw(painter, QPoint(0, 0));
        return;
    }

    painter->save();

    // 使用与主布局一致的参数创建临时布局，避免高亮的时候与选中的时候的布局不一致
    QTextLayout tempLayout(lineText);
    tempLayout.setFont(attribute<QFont>(kFont));
    QTextOption opt;
    opt.setAlignment((Qt::Alignment)attribute<uint>(kAlignment));
    opt.setWrapMode(QTextOption::NoWrap);
    opt.setTextDirection(attribute<Qt::LayoutDirection>(kTextDirection));
    tempLayout.setTextOption(opt);

    QList<QTextLayout::FormatRange> formats;
    int lineEnd = lineStartPos + lineText.length();
    for (const auto &match : allMatches) {
        int matchStart = match.first;
        int matchEnd = matchStart + match.second;
        if (matchEnd <= lineStartPos || matchStart >= lineEnd)
            continue;
        int highlightStart = qMax(matchStart, lineStartPos) - lineStartPos;
        int highlightEnd = qMin(matchEnd, lineEnd) - lineStartPos;
        int highlightLength = highlightEnd - highlightStart;
        if (highlightLength <= 0)
            continue;
        QTextLayout::FormatRange range;
        range.start = highlightStart;
        range.length = highlightLength;
        range.format.setForeground(highlightColor);
        formats.append(range);
    }
    tempLayout.setFormats(formats);

    tempLayout.beginLayout();
    QTextLine tempLine = tempLayout.createLine();
    if (tempLine.isValid()) {
        tempLine.setLineWidth(rect.width());
        tempLine.setPosition(rect.topLeft());
        tempLayout.endLayout();
        tempLayout.draw(painter, QPointF(0, 0));
    } else {
        tempLayout.endLayout();
        line.draw(painter, QPoint(0, 0));
    }

    painter->restore();
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
