/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
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

#include "itemdelegatehelper.h"

#include <QPainter>
#include <QApplication>
#include <QTextCodec>
#include <private/qtextengine_p.h>

#include <cmath>

#define CEIL(x) (static_cast<int>(std::ceil(x)))

/*!
 * \brief getIconPixmap 获取icon的pixmap
 *
 * \return QPixmap icon的pixmap图片
 **/
QPixmap ItemDelegateHelper::getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio = 1.0, QIcon::Mode mode, QIcon::State state)
{
    // ###(zccrs): 开启Qt::AA_UseHighDpiPixmaps后，QIcon::pixmap会自动执行 pixmapSize *= qApp->devicePixelRatio()
    //             而且，在有些QIconEngine的实现中，会去调用另一个QIcon::pixmap，导致 pixmapSize 在这种嵌套调用中越来越大
    //             最终会获取到一个是期望大小几倍的图片，由于图片太大，会很快将 QPixmapCache 塞满，导致后面再调用QIcon::pixmap
    //             读取新的图片时无法缓存，非常影响图片绘制性能。此处在获取图片前禁用 Qt::AA_UseHighDpiPixmaps，自行处理图片大小问题
    bool useHighDpiPixmaps = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps);
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, false);

    if (icon.isNull())
        return QPixmap();

    // 确保当前参数参入获取图片大小大于0
    if (size.width() <= 0 || size.height() <= 0)
        return QPixmap();

    QSize iconSize = icon.actualSize(size, mode, state);
    // 取出icon的真实大小
    QList<QSize> iconSizeList = icon.availableSizes();
    QSize iconRealSize;
    if (iconSizeList.count() > 0)
        iconRealSize = iconSizeList.first();
    else
        iconRealSize = iconSize;
    if (iconRealSize.width() <= 0 || iconRealSize.height() <= 0) {
        return icon.pixmap(iconSize);
    }

    // 确保特殊比例icon的高或宽不为0
    bool isSpecialSize = false;
    QSize tempSize(size.width(), size.height());
    while (iconSize.width() < 1) {
        tempSize.setHeight(tempSize.height() * 2);
        iconSize = icon.actualSize(tempSize, mode, state);
        isSpecialSize = true;
    }
    while (iconSize.height() < 1) {
        tempSize.setWidth(tempSize.width() * 2);
        iconSize = icon.actualSize(tempSize, mode, state);
        isSpecialSize = true;
    }

    if ((iconSize.width() > size.width() || iconSize.height() > size.height()) && !isSpecialSize)
        iconSize.scale(size, Qt::KeepAspectRatio);

    QSize pixmapSize = iconSize * pixelRatio;
    QPixmap px = icon.pixmap(pixmapSize, mode, state);

    // restore the value
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, useHighDpiPixmaps);

    // 约束特殊比例icon的尺寸
    if (isSpecialSize) {
        if (px.width() > size.width() * pixelRatio) {
            px = px.scaled(size.width() * CEIL(pixelRatio), px.height(), Qt::IgnoreAspectRatio);
        } else if (px.height() > size.height() * pixelRatio) {
            px = px.scaled(px.width(), size.height() * CEIL(pixelRatio), Qt::IgnoreAspectRatio);
        }
    }

    // 类型限定符的更改会导致缩放小数点丢失，从而引发缩放因子的bug
    if (px.width() > iconSize.width() * pixelRatio) {
        px.setDevicePixelRatio(px.width() / qreal(iconSize.width()));
    } else if (px.height() > iconSize.height() * pixelRatio) {
        px.setDevicePixelRatio(px.height() / qreal(iconSize.height()));
    } else {
        px.setDevicePixelRatio(pixelRatio);
    }

    return px;
}
/*!
 * \brief paintIcon 绘制指定区域内每一个icon的pixmap
 *
 * \return void
 **/
void ItemDelegateHelper::paintIcon(QPainter *painter, const QIcon &icon, const QRectF &rect, Qt::Alignment alignment, QIcon::Mode mode, QIcon::State state)
{
    // Copy of QStyle::alignedRect
    alignment = visualAlignment(painter->layoutDirection(), alignment);
    const qreal pixelRatio = painter->device()->devicePixelRatioF();
    const QPixmap &px = getIconPixmap(icon, rect.size().toSize(), pixelRatio, mode, state);
    qreal x = rect.x();
    qreal y = rect.y();
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += (rect.size().height() - h) / 2.0;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += rect.size().height() - h;
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += rect.size().width() - w;
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += (rect.size().width() - w) / 2.0;

    painter->drawPixmap(qRound(x), qRound(y), px);
}

QString ItemDelegateHelper::elideText(const QString &text, const QSizeF &size, QTextOption::WrapMode wordWrap, const QFont &font, Qt::TextElideMode mode, qreal lineHeight, qreal flags)
{
    QStringList lines;
    QTextLayout textLayout(text);

    textLayout.setFont(font);
    elideText(&textLayout, size, wordWrap, mode, lineHeight, static_cast<int>(flags), &lines);

    return lines.join('\n');
}

void ItemDelegateHelper::drawBackground(const qreal &backgroundRadius, const QRectF &rect, const QBrush &backgroundBrush, QPainter *painter)
{
    const QMarginsF margins(backgroundRadius, 0, backgroundRadius, 0);
    QRectF backBounding = rect;
    QPainterPath path;
    QRectF lastLineRect;

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

    bool isAntialiasing = painter->testRenderHint(QPainter::Antialiasing);
    qreal painterOpacity = painter->opacity();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setOpacity(1);
    painter->fillPath(path, backgroundBrush);
    painter->setRenderHint(QPainter::Antialiasing, isAntialiasing);
    painter->setOpacity(painterOpacity);
}

void ItemDelegateHelper::elideText(QTextLayout *layout, const QSizeF &size, QTextOption::WrapMode wordWrap, Qt::TextElideMode mode, qreal lineHeight, int flags, QStringList *lines, QPainter *painter, QPointF offset, const QColor &shadowColor, const QPointF &shadowOffset, const QBrush &background, qreal backgroundRadius, QList<QRectF> *boundingRegion)
{
    qreal height = 0;
    bool drawShadow = shadowColor.isValid();

    QString text = layout->engine()->hasFormats() ? layout->engine()->block.text() : layout->text();
    QTextOption &textOption = *const_cast<QTextOption *>(&layout->textOption());

    textOption.setWrapMode(wordWrap);

    if (flags & Qt::AlignRight)
        textOption.setAlignment(Qt::AlignRight);
    else if (flags & Qt::AlignHCenter)
        textOption.setAlignment(Qt::AlignHCenter);

    if (painter) {
        textOption.setTextDirection(painter->layoutDirection());
        layout->setFont(painter->font());
    } else {
        // dont paint
        layout->engine()->ignoreBidi = true;
    }

    layout->beginLayout();

    QTextLine line = layout->createLine();

    while (line.isValid()) {
        height += lineHeight;
        if (height + lineHeight > size.height()) {
            const QString &endString = layout->engine()->elidedText(mode, qRound(size.width()), flags, line.textStart());

            layout->endLayout();
            layout->setText(endString);

            if (layout->engine()->block.docHandle())
                const_cast<QTextDocument *>(layout->engine()->block.document())->setPlainText(endString);

            textOption.setWrapMode(QTextOption::NoWrap);
            layout->beginLayout();
            line = layout->createLine();
            line.setLineWidth(size.width() - 1);
            text = endString;
        } else {
            line.setLineWidth(size.width());
        }

        line.setPosition(offset);

        const QRectF rect = QRectF(line.naturalTextRect().topLeft(), QSizeF(line.naturalTextRect().width(), lineHeight));

        if (painter) {
            if (background.style() != Qt::NoBrush)
                drawBackground(backgroundRadius, rect, background, painter);

            if (drawShadow) {
                const QPen pen = painter->pen();

                painter->setPen(shadowColor);
                line.draw(painter, shadowOffset);
                // restore
                painter->setPen(pen);
            }

            line.draw(painter, QPointF(0, 0));
        }

        if (boundingRegion)
            boundingRegion->append(rect);

        offset.setY(offset.y() + lineHeight);

        if (lines)
            lines->append(text.mid(line.textStart(), line.textLength()));

        if (height + lineHeight > size.height())
            break;

        line = layout->createLine();
    }

    layout->endLayout();
}
