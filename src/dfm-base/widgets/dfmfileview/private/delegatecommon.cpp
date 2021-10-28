/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#include "delegatecommon.h"

#include <DArrowRectangle>
#include <DListView>
#include <DArrowRectangle>
#include <DPalette>
#include <DApplicationHelper>

#include <private/qtextengine_p.h>

DWIDGET_USE_NAMESPACE

namespace GlobalPrivate {

    QRectF boundingRect(const QList<QRectF> &rects)
    {
        QRectF bounding;

        if (rects.isEmpty())
            return bounding;

        bounding = rects.first();

        for (const QRectF &r : rects) {
            if (r.top() < bounding.top()) {
                bounding.setTop(r.top());
            }

            if (r.left() < bounding.left()) {
                bounding.setLeft(r.left());
            }

            if (r.right() > bounding.right()) {
                bounding.setRight(r.right());
            }

            if (r.bottom() > bounding.bottom()) {
                bounding.setBottom(r.bottom());
            }
        }

        return bounding;
    }

    QPainterPath boundingPath(QList<QRectF> rects, qreal radius, qreal padding)
    {
        QPainterPath path;
        const QMarginsF margins(radius + padding, 0, radius + padding, 0);

        if (rects.count() == 1) {
            path.addRoundedRect(rects.first().marginsAdded(margins).adjusted(0, -padding, 0, padding), radius, radius);

            return path;
        }

        auto joinRightCorner = [&](const QRectF & rect, const QRectF & prevRect, const QRectF & nextRect) {
            if (Q_LIKELY(prevRect.isValid())) {
                qreal new_radius = qMin(radius, qAbs(prevRect.right() - rect.right()) / 2);

                if (rect.right() > prevRect.right()) {
                    path.arcTo(rect.right() - new_radius * 2, rect.y() - padding, new_radius * 2, new_radius * 2, 90, -90);
                } else if (rect.right() < prevRect.right()) {
                    path.arcTo(rect.right(), rect.y() + padding, new_radius * 2, new_radius * 2, 90, 90);
                }
            } else {
                path.arcTo(rect.right() - radius * 2, rect.y() - padding, radius * 2, radius * 2, 90, -90);
            }

            if (Q_LIKELY(nextRect.isValid())) {
                qreal new_radius = qMin(radius, qAbs(nextRect.right() - rect.right()) / 2);

                if (rect.right() > nextRect.right()) {
                    path.arcTo(rect.right() - new_radius * 2, rect.bottom() - new_radius * 2 + padding, new_radius * 2, new_radius * 2, 0, -90);
                } else if (rect.right() < nextRect.right()) {
                    path.arcTo(rect.right(), rect.bottom() - new_radius * 2 - padding, new_radius * 2, new_radius * 2, 180, 90);
                }
            } else {
                path.arcTo(rect.right() - radius * 2, rect.bottom() - radius * 2 + padding, radius * 2, radius * 2, 0, -90);
            }
        };

        auto joinLeftCorner = [&](const QRectF & rect, const QRectF & prevRect, const QRectF & nextRect) {
            if (Q_LIKELY(nextRect.isValid())) {
                qreal new_radius = qMin(radius, qAbs(nextRect.x() - rect.x()) / 2);

                if (rect.x() > nextRect.x()) {
                    path.arcTo(rect.x() - new_radius * 2, rect.bottom() - new_radius * 2 - padding, new_radius * 2, new_radius * 2, 270, 90);
                } else if (rect.x() < nextRect.x()) {
                    path.arcTo(rect.x(), rect.bottom() - new_radius * 2 + padding, new_radius * 2, new_radius * 2, 270, -90);
                }
            } else {
                path.arcTo(rect.x(), rect.bottom() - radius * 2 + padding, radius * 2, radius * 2, 270, -90);
            }

            if (Q_LIKELY(prevRect.isValid())) {
                qreal new_radius = qMin(radius, qAbs(prevRect.x() - rect.x()) / 2);

                if (rect.x() > prevRect.x()) {
                    path.arcTo(rect.x() - new_radius * 2, rect.y() + padding, new_radius * 2, new_radius * 2, 0, 90);
                } else if (rect.x() < prevRect.x()) {
                    path.arcTo(rect.x(), rect.y() - padding, new_radius * 2, new_radius * 2, 180, -90);
                }
            } else {
                path.arcTo(rect.x(), rect.y() - padding, radius * 2, radius * 2, 180, -90);
            }
        };

        auto preproccess = [&](QRectF & rect, const QRectF & prev) {
            if (qAbs(rect.x() - prev.x()) < radius) {
                rect.setLeft(prev.x());
            }

            if (qAbs(rect.right() - prev.right()) < radius) {
                rect.setRight(prev.right());
            }
        };

        for (int i = 1; i < rects.count(); ++i) {
            preproccess(rects[i], rects.at(i - 1));
        }

        const QRectF &first = rects.first().marginsAdded(margins);

        path.arcMoveTo(first.right() - radius * 2, first.y() - padding, radius * 2, radius * 2, 90);
        joinRightCorner(first, QRectF(), rects.at(1).marginsAdded(margins));

        for (int i = 1; i < rects.count() - 1; ++i) {
            joinRightCorner(rects.at(i) + margins, rects.at(i - 1).marginsAdded(margins), rects.at(i + 1).marginsAdded(margins));
        }

        QRectF last = rects.last();
        const QRectF &prevRect = rects.at(rects.count() - 2);

        joinRightCorner(last.marginsAdded(margins), prevRect.marginsAdded(margins), QRectF());
        joinLeftCorner(last.marginsAdded(margins), prevRect.marginsAdded(margins), QRectF());

        for (int i = rects.count() - 2; i > 0; --i) {
            joinLeftCorner(rects.at(i) + margins, rects.at(i - 1).marginsAdded(margins), rects.at(i + 1).marginsAdded(margins));
        }

        joinLeftCorner(first, QRectF(), rects.at(1).marginsAdded(margins));

        path.closeSubpath();

        return path;
    }

    void elideText(QTextLayout *layout, const QSizeF &size, QTextOption::WrapMode wordWrap, Qt::TextElideMode mode, qreal lineHeight, int flags, QStringList *lines, QPainter *painter, QPointF offset, const QColor &shadowColor, const QPointF &shadowOffset, const QBrush &background, qreal backgroundRadius, QList<QRectF> *boundingRegion)
    {
        qreal height = 0;
        bool drawBackground = background.style() != Qt::NoBrush;
        bool drawShadow = shadowColor.isValid();

        QString text = layout->engine()->hasFormats() ? layout->engine()->block.text() : layout->text();
        QTextOption &text_option = *const_cast<QTextOption *>(&layout->textOption());

        text_option.setWrapMode(wordWrap);

        if (flags & Qt::AlignRight)
            text_option.setAlignment(Qt::AlignRight);
        else if (flags & Qt::AlignHCenter)
            text_option.setAlignment(Qt::AlignHCenter);

        if (painter) {
            text_option.setTextDirection(painter->layoutDirection());
            layout->setFont(painter->font());
        } else {
            // dont paint
            layout->engine()->ignoreBidi = true;
        }

        auto naturalTextRect = [&](const QRectF rect) {
            QRectF new_rect = rect;

            new_rect.setHeight(lineHeight);

            return new_rect;
        };

        auto drawShadowFun = [&](const QTextLine & line) {
            const QPen pen = painter->pen();

            painter->setPen(shadowColor);
            line.draw(painter, shadowOffset);

            // restore
            painter->setPen(pen);
        };

        layout->beginLayout();

        QTextLine line = layout->createLine();
        QRectF lastLineRect;

        while (line.isValid()) {
            height += lineHeight;
            if (height + lineHeight > size.height()) {
                const QString &end_str = layout->engine()->elidedText(mode, qRound(size.width()), flags, line.textStart());

                layout->endLayout();
                layout->setText(end_str);

                if (layout->engine()->block.docHandle()) {
                    const_cast<QTextDocument *>(layout->engine()->block.document())->setPlainText(end_str);
                }

                text_option.setWrapMode(QTextOption::NoWrap);
                layout->beginLayout();
                line = layout->createLine();
                line.setLineWidth(size.width() - 1);
                text = end_str;
            } else {
                line.setLineWidth(size.width());
            }

            line.setPosition(offset);

            const QRectF rect = naturalTextRect(line.naturalTextRect());

            if (painter) {
                if (drawBackground) {
                    const QMarginsF margins(backgroundRadius, 0, backgroundRadius, 0);
                    QRectF backBounding = rect;
                    QPainterPath path;

                    if (lastLineRect.isValid()) {
                        if (qAbs(rect.width() - lastLineRect.width()) < backgroundRadius * 2) {
                            backBounding.setWidth(lastLineRect.width());
                            backBounding.moveCenter(rect.center());
                            path.moveTo(lastLineRect.x() - backgroundRadius, lastLineRect.bottom() - backgroundRadius);
                            path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
                            path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
                            path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius);
                            path.lineTo(lastLineRect.right() + backgroundRadius, backBounding.bottom() - backgroundRadius);
                            path.arcTo(backBounding.right() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 0, -90);
                            path.lineTo(backBounding.x(), backBounding.bottom());
                            path.arcTo(backBounding.x() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, -90);
                            lastLineRect = backBounding;
                        } else if (lastLineRect.width() > rect.width()) {
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
                            lastLineRect = rect;
                        } else {
                            backBounding += margins;
                            path.moveTo(lastLineRect.x() - backgroundRadius * 2, lastLineRect.bottom());
                            path.arcTo(lastLineRect.x() - backgroundRadius * 3, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, 90);
                            path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
                            path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
                            path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2);
                            path.arcTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 180, 90);
                            path.addRoundedRect(backBounding, backgroundRadius, backgroundRadius);
                            lastLineRect = rect;
                        }
                    } else {
                        lastLineRect = backBounding;
                        path.addRoundedRect(backBounding + margins, backgroundRadius, backgroundRadius);
                    }

                    bool a = painter->testRenderHint(QPainter::Antialiasing);
                    qreal o = painter->opacity();

                    painter->setRenderHint(QPainter::Antialiasing, true);
                    painter->setOpacity(1);
                    painter->fillPath(path, background);
                    painter->setRenderHint(QPainter::Antialiasing, a);
                    painter->setOpacity(o);
                }

                if (drawShadow) {
                    drawShadowFun(line);
                }

                line.draw(painter, QPointF(0, 0));
            }

            if (boundingRegion) {
                boundingRegion->append(rect);
            }

            offset.setY(offset.y() + lineHeight);

            if (lines) {
                lines->append(text.mid(line.textStart(), line.textLength()));
            }

            if (height + lineHeight > size.height())
                break;

            line = layout->createLine();
        }

        layout->endLayout();
    }

    QString elideText(const QString &text, const QSizeF &size, QTextOption::WrapMode wordWrap, const QFont &font, Qt::TextElideMode mode, qreal lineHeight, qreal flags)
    {
        QTextLayout textLayout(text);

        textLayout.setFont(font);

        QStringList lines;

        elideText(&textLayout, size, wordWrap, mode, lineHeight, static_cast<int>(flags), &lines);

        return lines.join('\n');
    }

    QString replaceFileName(QString name, QString chars)
    {
        if (name.isEmpty() || chars.isEmpty())
            return name;

        return name.remove(QRegularExpression(chars));
    }

    void showAlertMessage(QPoint globalPoint, const QColor &backgroundColor, const QString &text, int duration)
    {
        static DArrowRectangle* tooltip = nullptr;
        if (!tooltip) {
            tooltip = new DArrowRectangle(DArrowRectangle::ArrowTop, nullptr);
            tooltip->setObjectName("AlertTooltip");
            QLabel *label = new QLabel(tooltip);
            label->setWordWrap(true);
            label->setMaximumWidth(500);
            tooltip->setContent(label);
            tooltip->setBackgroundColor(backgroundColor);
            tooltip->setArrowX(15);
            tooltip->setArrowHeight(5);

            QTimer::singleShot(duration, [=] {
                delete tooltip;
                tooltip = nullptr;
            });

            label->setText(text);
            label->adjustSize();

            tooltip->show(static_cast<int>(globalPoint.x()),static_cast<int>(globalPoint.y()));
        }
    }

    void setFileNameReplaceChars(const QString &rep)
    {
        replaceChars = rep;
    }

    QString fileNameReplaceChars(){
        return replaceChars;
    }

} // namespace GlobalPrivate
