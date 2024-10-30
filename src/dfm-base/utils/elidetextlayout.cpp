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
    auto processLine = [this, &ret, painter, &lastLineRect, background, textLineHeight, &curText, textLines](QTextLine &line) {
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

            // draw text line
            line.draw(painter, QPoint(0, 0));
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
                    elideText = fm.elidedText(text().mid(line.textStart()), elideMode, qRound(size.width()));
                    curText = elideText;
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

void ElideTextLayout::initLayoutOption(QTextLayout *lay)
{
    auto opt = lay->textOption();
    opt.setAlignment((Qt::Alignment)attribute<uint>(kAlignment));
    opt.setWrapMode((QTextOption::WrapMode)attribute<uint>(kWrapMode));
    opt.setTextDirection(attribute<Qt::LayoutDirection>(kTextDirection));
    lay->setTextOption(opt);
    lay->setFont(attribute<QFont>(kFont));
}
