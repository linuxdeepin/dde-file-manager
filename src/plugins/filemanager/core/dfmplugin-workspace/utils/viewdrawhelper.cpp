/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "viewdrawhelper.h"
#include "views/fileview.h"
#include "views/iconitemdelegate.h"
#include "models/filesortfilterproxymodel.h"
#include "utils/itemdelegatehelper.h"

#include <QPainter>

using namespace dfmbase::Global;
using namespace GlobalPrivate;
using namespace dfmplugin_workspace;

ViewDrawHelper::ViewDrawHelper(FileView *parent)
    : view(parent)
{
}

QPixmap ViewDrawHelper::renderDragPixmap(dfmbase::Global::ViewMode mode, QModelIndexList indexes) const
{
    if (indexes.isEmpty())
        return QPixmap();

    const int dragCount = indexes.length();
    QModelIndex topIndex = view->currentPressIndex();
    if (!topIndex.isValid())
        topIndex = indexes.first();
    indexes.removeAll(topIndex);

    const qreal scale = view->devicePixelRatioF();
    QStyleOptionViewItem option = view->viewOptions();
    option.state |= QStyle::State_Selected;

    if (mode == ViewMode::kIconMode) {
        option.rect = option.rect.translated(kDragIconOutline, kDragIconOutline);

        int pixmapSize = kDragIconSize + kDragIconOutline * 2;
        QRect pixRect(0, 0, pixmapSize, pixmapSize);

        QPixmap pixmap(pixRect.size() * scale);
        pixmap.setDevicePixelRatio(scale);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);

        drawDragIcons(&painter, option, pixRect, indexes, topIndex);
        drawDragCount(&painter, topIndex, option, dragCount);

        return pixmap;
    } else if (mode == ViewMode::kListMode) {
        option.rect = option.rect.translated((kListDragIconSize - kDragIconSize) / 2, 0);

        QRect pixRect(0, 0, kListDragIconSize, kDragIconSize + kListDragTextHeight);
        QPixmap pixmap(pixRect.size() * scale);
        pixmap.setDevicePixelRatio(scale);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);

        if (indexes.isEmpty()) {
            drawDragIcons(&painter, option, pixRect, indexes, topIndex);
            drawDragText(&painter, topIndex);
        } else {
            drawDragIcons(&painter, option, pixRect, indexes, topIndex);
            drawDragCount(&painter, topIndex, option, dragCount);
        }

        return pixmap;
    }

    return QPixmap();
}

void ViewDrawHelper::drawDragIcons(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QModelIndexList &indexes, const QModelIndex &topIndex) const
{
    QSize defaultIconSize = QSize(kDragIconSize, kDragIconSize);

    qreal offsetX = rect.width() / 2;
    qreal offsetY = rect.height() / 2;

    for (int i = qMin(kDragIconMax - 1, indexes.length() - 1); i >= 0; --i) {
        painter->setOpacity(1.0 - (i + 5) * kDragIconOpacity);

        // rotate
        qreal rotate = kDragIconRotate * (qRound((i + 1.0) / 2.0) / 2.0 + 1.0) * (i % 2 == 1 ? -1 : 1);
        painter->translate(offsetX, offsetY);
        painter->rotate(rotate);
        painter->translate(-offsetX, -offsetY);

        view->itemDelegate()->paintDragIcon(painter, option, indexes.at(i), defaultIconSize);

        // reset rotation
        painter->translate(offsetX, offsetY);
        painter->rotate(-rotate);
        painter->translate(-offsetX, -offsetY);
    }

    //draw top icon
    painter->setOpacity(0.8);
    view->itemDelegate()->paintDragIcon(painter, option, topIndex, defaultIconSize);
}

void ViewDrawHelper::drawDragCount(QPainter *painter, const QModelIndex &topIndex, const QStyleOptionViewItem &option, int count) const
{
    QSize defaultIconSize = QSize(kDragIconSize, kDragIconSize);
    int length = count > kDragIconMaxCount ? 28 : 24;   //diffrent size for diffrent number of chars
    QSize iconRealSize = view->itemDelegate()->getIndexIconSize(option, topIndex, defaultIconSize);
    if (iconRealSize.width() > defaultIconSize.width() || iconRealSize.height() > defaultIconSize.height())
        iconRealSize.scale(defaultIconSize, Qt::KeepAspectRatio);
    int x = kDragIconOutline + (kDragIconSize + iconRealSize.width() - length) / 2;
    int y = kDragIconOutline + (kDragIconSize + iconRealSize.height() - length) / 2;

    QColor pointColor(244, 74, 74);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setOpacity(1);
    painter->setPen(pointColor);
    painter->setBrush(pointColor);
    painter->drawEllipse(x, y, length, length);

    painter->setPen(Qt::white);
    QFont ft("Arial");
    ft.setPixelSize(12);
    ft.setBold(true);
    painter->setFont(ft);
    QString countStr = count > kDragIconMaxCount ? QString::number(kDragIconMaxCount).append("+") : QString::number(count);
    painter->drawText(QRect(x, y, length, length), Qt::AlignCenter, countStr);
}

void ViewDrawHelper::drawDragText(QPainter *painter, const QModelIndex &index) const
{
    QString fileName = view->model()->data(index, ItemRoles::kItemFileDisplayNameRole).toString();

    QTextLayout layout;
    layout.setText(fileName);
    layout.setFont(painter->font());
    painter->setPen(Qt::white);

    QRectF boundingRect((kListDragIconSize - kListDragTextWidth) / 2, kDragIconSize, kListDragTextWidth, kListDragTextHeight);
    QTextOption::WrapMode wordWrap(QTextOption::WrapAtWordBoundaryOrAnywhere);
    Qt::TextElideMode mode(Qt::ElideLeft);
    int textLineHeight = view->fontMetrics().lineSpacing();
    int flags = Qt::AlignHCenter;
    QColor shadowColor;
    QBrush background(view->palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Highlight));

    ItemDelegateHelper::elideText(&layout, boundingRect.size(), wordWrap, mode, textLineHeight, flags, nullptr,
                                  painter, boundingRect.topLeft(), shadowColor, QPointF(0, 1),
                                  background);
}
