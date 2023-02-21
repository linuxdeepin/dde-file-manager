// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "viewdrawhelper.h"
#include "views/fileview.h"
#include "views/iconitemdelegate.h"
#include "models/filesortfilterproxymodel.h"
#include "utils/itemdelegatehelper.h"

#include <QPainter>

using namespace dfmbase;
using namespace dfmbase::Global;
using namespace GlobalPrivate;
using namespace dfmplugin_workspace;

ViewDrawHelper::ViewDrawHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
}

QPixmap ViewDrawHelper::renderDragPixmap(dfmbase::Global::ViewMode mode, QModelIndexList indexes)
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
        QRectF rect = view->visualRect(topIndex);
        QRectF iconRect = view->itemDelegate()->itemIconRect(rect);
        dragIconSize = iconRect.width();

        option.rect = option.rect.translated(kDragIconOutline, kDragIconOutline);
        int pixmapSize = dragIconSize + kDragIconOutline * 2;

        QRect pixRect(0, 0, pixmapSize, pixmapSize);

        QPixmap pixmap(pixRect.size() * scale);
        pixmap.setDevicePixelRatio(scale);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);

        drawDragIcons(&painter, option, pixRect, indexes, topIndex);
        if (dragCount != 1)
            drawDragCount(&painter, topIndex, option, dragCount);

        return pixmap;
    } else if (mode == ViewMode::kListMode) {
        dragIconSize = kDragIconSize;
        option.rect = option.rect.translated((kListDragIconSize - dragIconSize) / 2, 0);

        QRect pixRect(0, 0, kListDragIconSize, dragIconSize + kListDragTextHeight);
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
    QSize defaultIconSize = QSize(dragIconSize, dragIconSize);

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
    QSize defaultIconSize = QSize(dragIconSize, dragIconSize);
    int length = count > kDragIconMaxCount ? 28 : 24;   //diffrent size for diffrent number of chars
    QSize iconRealSize = view->itemDelegate()->getIndexIconSize(option, topIndex, defaultIconSize);
    if (iconRealSize.width() > defaultIconSize.width() || iconRealSize.height() > defaultIconSize.height())
        iconRealSize.scale(defaultIconSize, Qt::KeepAspectRatio);
    int x = kDragIconOutline + (dragIconSize + iconRealSize.width() - length) / 2 - 10;
    int y = kDragIconOutline + (dragIconSize + iconRealSize.height() - length) / 2 - 10;

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
    painter->setPen(Qt::white);

    QString fileName = view->model()->data(index, ItemRoles::kItemFileDisplayNameRole).toString();
    QRectF boundingRect((kListDragIconSize - kListDragTextWidth) / 2, dragIconSize, kListDragTextWidth, kListDragTextHeight);
    QTextOption::WrapMode wordWrap(QTextOption::WrapAtWordBoundaryOrAnywhere);
    Qt::TextElideMode mode(Qt::ElideLeft);
    int textLineHeight = view->fontMetrics().lineSpacing();
    int flags = Qt::AlignHCenter;
    QBrush background(view->palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Highlight));

    QScopedPointer<ElideTextLayout> layout(ItemDelegateHelper::createTextLayout(fileName, wordWrap, textLineHeight, flags, painter));
    layout->setAttribute(ElideTextLayout::kBackgroundRadius, kIconModeRectRadius);

    layout->layout(boundingRect, mode, painter, background);
}
